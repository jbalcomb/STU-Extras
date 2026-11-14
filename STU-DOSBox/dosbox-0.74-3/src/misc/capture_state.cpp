/*
 *  DOSBox state-capture module (v1).
 *
 *  Periodically polls a user-configured list of guest-memory addresses
 *  via the PIC scheduler and appends one CSV row per sample to a host
 *  file.  See the [capture] section in the generated dosbox config for
 *  the input format.
 *
 *  Probe forms:
 *    Direct:        LABEL : SEG : OFF : WIDTH
 *    Indirect near: LABEL : ptr    : SEG : POFF : DELTA : WIDTH
 *    Indirect far:  LABEL : farptr : SEG : POFF : DELTA : WIDTH
 *
 *  SEG may be a 4-digit hex literal or the keyword `ds`, in which case
 *  the live DS register value is used at sample time.
 *
 *  Indirect near: read a 16-bit pointer at SEG:POFF, then sample WIDTH
 *  bytes at SEG:(pointer+DELTA).
 *
 *  Indirect far: read a 32-bit far pointer at SEG:POFF (little-endian:
 *  offset then segment), then sample WIDTH bytes at (resolved_seg :
 *  resolved_off + DELTA).  Used for arrays in a different segment than
 *  the pointer (typical for Borland C++ large-model DOS binaries).
 *
 *  Optional sentinel guard:
 *    verify = SEG : OFF : ASCII_BYTES
 *  Multiple verify lines AND'ed.  If any check fails on a given tick
 *  (e.g. DS happens to be a BIOS/DOS segment, not the program's),
 *  the row is dropped entirely.  Use this to gate `ds`-symbolic probes
 *  on a stable program-identifying string in the data segment.
 */

#include "dosbox.h"
#include "control.h"
#include "setup.h"
#include "pic.h"
#include "mem.h"
#include "regs.h"
#include "timer.h"
#include "logging.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

enum ProbeWidth { PW_BYTE, PW_WORD, PW_DWORD };
enum ProbeKind  { PK_DIRECT, PK_INDIRECT_NEAR, PK_INDIRECT_FAR, PK_REG };
enum SegRef     { SR_LITERAL, SR_DS };
enum EmitMode   { EM_ALWAYS, EM_ON_CHANGE };

/* For PK_REG probes -- which CPU register to read at sample time. */
enum RegId {
	RG_CS, RG_DS, RG_ES, RG_FS, RG_GS, RG_SS,
	RG_IP, RG_AX, RG_BX, RG_CX, RG_DX, RG_SI, RG_DI, RG_BP, RG_SP
};

struct Probe {
	std::string label;
	ProbeKind   kind;
	SegRef      seg_ref;
	Bit16u      seg_literal;   // used when seg_ref == SR_LITERAL
	Bit16u      off;           // direct: target offset; indirect: pointer offset
	Bit16u      delta;         // indirect only: added to dereferenced pointer
	ProbeWidth  width;
	RegId       reg;           // PK_REG only
};

struct Verifier {
	SegRef      seg_ref;
	Bit16u      seg_literal;
	Bit16u      offset;
	std::string expected;      // raw bytes to compare against
	bool        negate;        // false: must match (verify); true: must NOT match (denyif)
};

static std::vector<Probe>    probes;
static std::vector<Verifier> verifiers;
static EmitMode              emit_mode = EM_ALWAYS;
static std::vector<Bit32u>   last_values;
static bool                  has_emitted = false;
static FILE* capfile = NULL;
static float tick_ms = 0.0f;
static Bit32u sample_idx = 0;
static bool tick_scheduled = false;

static std::string trim(const std::string& s) {
	size_t a = s.find_first_not_of(" \t\r");
	if (a == std::string::npos) return "";
	size_t b = s.find_last_not_of(" \t\r");
	return s.substr(a, b - a + 1);
}

static bool parse_uint16_hex(const std::string& s, Bit16u* out) {
	if (s.empty()) return false;
	const char* p = s.c_str();
	char* end = NULL;
	unsigned long v = strtoul(p, &end, 16);
	if (end == p || *end != '\0' || v > 0xFFFFul) return false;
	*out = (Bit16u)v;
	return true;
}

static bool parse_seg(const std::string& s, SegRef* sr, Bit16u* lit) {
	if (s == "ds" || s == "DS") {
		*sr = SR_DS;
		*lit = 0;
		return true;
	}
	if (parse_uint16_hex(s, lit)) {
		*sr = SR_LITERAL;
		return true;
	}
	return false;
}

static bool parse_reg(const std::string& s, RegId* r) {
	if      (s == "cs") *r = RG_CS;
	else if (s == "ds") *r = RG_DS;
	else if (s == "es") *r = RG_ES;
	else if (s == "fs") *r = RG_FS;
	else if (s == "gs") *r = RG_GS;
	else if (s == "ss") *r = RG_SS;
	else if (s == "ip") *r = RG_IP;
	else if (s == "ax") *r = RG_AX;
	else if (s == "bx") *r = RG_BX;
	else if (s == "cx") *r = RG_CX;
	else if (s == "dx") *r = RG_DX;
	else if (s == "si") *r = RG_SI;
	else if (s == "di") *r = RG_DI;
	else if (s == "bp") *r = RG_BP;
	else if (s == "sp") *r = RG_SP;
	else return false;
	return true;
}

static Bit32u read_reg(RegId r) {
	switch (r) {
	case RG_CS: return SegValue(cs);
	case RG_DS: return SegValue(ds);
	case RG_ES: return SegValue(es);
	case RG_FS: return SegValue(fs);
	case RG_GS: return SegValue(gs);
	case RG_SS: return SegValue(ss);
	case RG_IP: return reg_ip;
	case RG_AX: return reg_ax;
	case RG_BX: return reg_bx;
	case RG_CX: return reg_cx;
	case RG_DX: return reg_dx;
	case RG_SI: return reg_si;
	case RG_DI: return reg_di;
	case RG_BP: return reg_bp;
	case RG_SP: return reg_sp;
	}
	return 0;
}

static bool parse_width(const std::string& s, ProbeWidth* w) {
	if      (s == "byte")  *w = PW_BYTE;
	else if (s == "word")  *w = PW_WORD;
	else if (s == "dword") *w = PW_DWORD;
	else return false;
	return true;
}

static bool parse_probe(const std::string& spec, Probe& out) {
	std::vector<std::string> fields;
	size_t start = 0;
	for (size_t i = 0; i <= spec.size(); ++i) {
		if (i == spec.size() || spec[i] == ':') {
			fields.push_back(trim(spec.substr(start, i - start)));
			start = i + 1;
		}
	}
	/* Reg probes have 3 fields, indirect have 6, direct have 4 -- the per-
	 * branch checks below enforce the exact count; here we only require at
	 * least 3 fields and a non-empty label. */
	if (fields.size() < 3 || fields[0].empty()) return false;
	out.label = fields[0];

	if (fields[1] == "ptr" || fields[1] == "farptr") {
		// Indirect: LABEL : (ptr|farptr) : SEG : POFF : DELTA : WIDTH
		if (fields.size() != 6) return false;
		out.kind = (fields[1] == "ptr") ? PK_INDIRECT_NEAR : PK_INDIRECT_FAR;
		if (!parse_seg(fields[2], &out.seg_ref, &out.seg_literal)) return false;
		if (!parse_uint16_hex(fields[3], &out.off))   return false;
		if (!parse_uint16_hex(fields[4], &out.delta)) return false;
		if (!parse_width(fields[5], &out.width))      return false;
	} else if (fields[1] == "reg") {
		// CPU register read: LABEL : reg : NAME
		if (fields.size() != 3) return false;
		out.kind  = PK_REG;
		out.width = PW_WORD;
		if (!parse_reg(fields[2], &out.reg)) return false;
	} else {
		// Direct: LABEL : SEG : OFF : WIDTH
		if (fields.size() != 4) return false;
		out.kind = PK_DIRECT;
		if (!parse_seg(fields[1], &out.seg_ref, &out.seg_literal)) return false;
		if (!parse_uint16_hex(fields[2], &out.off))   return false;
		out.delta = 0;
		if (!parse_width(fields[3], &out.width))      return false;
	}
	return true;
}

static Bit16u probe_seg(const Probe& p) {
	return (p.seg_ref == SR_DS) ? SegValue(ds) : p.seg_literal;
}

static bool verifiers_pass() {
	for (size_t i = 0; i < verifiers.size(); ++i) {
		const Verifier& v = verifiers[i];
		Bit16u seg = (v.seg_ref == SR_DS) ? SegValue(ds) : v.seg_literal;
		bool matched = true;
		for (size_t j = 0; j < v.expected.size(); ++j) {
			if (real_readb(seg, (Bit16u)(v.offset + j)) !=
			    (Bit8u)v.expected[j]) {
				matched = false;
				break;
			}
		}
		// verify  (negate=false): must match -> fail if not matched
		// denyif  (negate=true ): must NOT match -> fail if matched
		if (v.negate ? matched : !matched) return false;
	}
	return true;
}

static bool parse_verify(const std::string& val, Verifier& out) {
	// Format: SEG : OFF : ASCII_BYTES
	// Split on first two colons; everything after the second colon is
	// the expected byte sequence (preserved verbatim after trimming).
	// Caller sets `negate` based on which key (`verify` vs `denyif`).
	size_t c1 = val.find(':');
	if (c1 == std::string::npos) return false;
	size_t c2 = val.find(':', c1 + 1);
	if (c2 == std::string::npos) return false;
	std::string seg_s = trim(val.substr(0, c1));
	std::string off_s = trim(val.substr(c1 + 1, c2 - c1 - 1));
	std::string bytes = trim(val.substr(c2 + 1));
	if (!parse_seg(seg_s, &out.seg_ref, &out.seg_literal)) return false;
	if (!parse_uint16_hex(off_s, &out.offset)) return false;
	if (bytes.empty()) return false;
	out.expected = bytes;
	out.negate = false;
	return true;
}

static Bit32u read_probe(const Probe& p) {
	if (p.kind == PK_REG) return read_reg(p.reg);

	Bit16u seg = probe_seg(p);
	Bit16u off = p.off;
	if (p.kind == PK_INDIRECT_NEAR) {
		Bit16u ptr = real_readw(seg, p.off);
		off = (Bit16u)(ptr + p.delta);
	} else if (p.kind == PK_INDIRECT_FAR) {
		Bit32u fptr = real_readd(seg, p.off);
		seg = (Bit16u)(fptr >> 16);
		off = (Bit16u)((fptr & 0xFFFF) + p.delta);
	}
	switch (p.width) {
	case PW_BYTE:  return real_readb(seg, off);
	case PW_WORD:  return real_readw(seg, off);
	case PW_DWORD: return real_readd(seg, off);
	}
	return 0;
}

static void capture_tick(Bitu /*val*/) {
	if (capfile && verifiers_pass()) {
		// Read all probes first so emit decisions see consistent values.
		std::vector<Bit32u> values;
		values.reserve(probes.size());
		for (size_t i = 0; i < probes.size(); ++i)
			values.push_back(read_probe(probes[i]));

		bool should_emit = true;
		if (emit_mode == EM_ON_CHANGE && has_emitted) {
			should_emit = false;
			for (size_t i = 0; i < values.size(); ++i) {
				/* PK_REG probes (CS/IP/AX/etc.) change on every CPU
				 * instruction -- skip them for change-detection or
				 * on_change becomes always-emit.  They're still read
				 * into `values` and written to the row whenever
				 * something else triggers the emit. */
				if (probes[i].kind == PK_REG) continue;
				if (values[i] != last_values[i]) {
					should_emit = true;
					break;
				}
			}
		}

		if (should_emit) {
			fprintf(capfile, "%u,%u",
			        (unsigned)sample_idx, (unsigned)GetTicks());
			for (size_t i = 0; i < values.size(); ++i)
				fprintf(capfile, ",%u", (unsigned)values[i]);
			fputc('\n', capfile);
			fflush(capfile);
			last_values = values;
			has_emitted = true;
		}
	}
	// sample_idx ticks unconditionally so gaps in the CSV reflect
	// how often the verifier(s) rejected or change-detection suppressed.
	if (capfile) sample_idx++;
	PIC_AddEvent(capture_tick, tick_ms);
}

static void capture_destroy(Section*) {
	if (tick_scheduled) {
		PIC_RemoveEvents(capture_tick);
		tick_scheduled = false;
	}
	if (capfile) {
		fclose(capfile);
		capfile = NULL;
	}
	probes.clear();
	verifiers.clear();
	last_values.clear();
	has_emitted = false;
	emit_mode = EM_ALWAYS;
	sample_idx = 0;
}

static void capture_init(Section* sec) {
	Section_line* line_sec = static_cast<Section_line*>(sec);
	sec->AddDestroyFunction(&capture_destroy);

	std::string capturefile;
	int rate_hz = 60;
	const std::string& data = line_sec->data;
	size_t pos = 0;
	while (pos < data.size()) {
		size_t eol = data.find('\n', pos);
		if (eol == std::string::npos) eol = data.size();
		std::string ln = trim(data.substr(pos, eol - pos));
		pos = eol + 1;
		if (ln.empty() || ln[0] == '#') continue;

		size_t eq = ln.find('=');
		if (eq != std::string::npos) {
			std::string key = trim(ln.substr(0, eq));
			std::string val = trim(ln.substr(eq + 1));
			if (key == "capturefile") {
				capturefile = val;
			} else if (key == "rate_hz") {
				rate_hz = atoi(val.c_str());
				if (rate_hz < 1) rate_hz = 1;
				if (rate_hz > 100000) rate_hz = 100000;
			} else if (key == "emit_mode") {
				if      (val == "always")    emit_mode = EM_ALWAYS;
				else if (val == "on_change") emit_mode = EM_ON_CHANGE;
				else LOG_MSG("CAPTURE: bad emit_mode '%s' (expected always|on_change)",
				             val.c_str());
			} else if (key == "verify" || key == "denyif") {
				Verifier v;
				if (parse_verify(val, v)) {
					v.negate = (key == "denyif");
					verifiers.push_back(v);
				} else {
					LOG_MSG("CAPTURE: bad %s '%s'",
					        key.c_str(), val.c_str());
				}
			} else {
				LOG_MSG("CAPTURE: unknown key '%s'", key.c_str());
			}
		} else {
			Probe p;
			if (parse_probe(ln, p)) {
				probes.push_back(p);
			} else {
				LOG_MSG("CAPTURE: ignoring malformed probe '%s'",
				        ln.c_str());
			}
		}
	}

	if (capturefile.empty()) return;

	capfile = fopen(capturefile.c_str(), "w");
	if (!capfile) {
		LOG_MSG("CAPTURE: cannot open '%s' for writing",
		        capturefile.c_str());
		probes.clear();
		return;
	}

	fprintf(capfile, "sample,wall_ms");
	for (size_t i = 0; i < probes.size(); ++i)
		fprintf(capfile, ",%s", probes[i].label.c_str());
	fputc('\n', capfile);
	fflush(capfile);

	tick_ms = 1.0e3f / (float)rate_hz;
	sample_idx = 0;
	PIC_AddEvent(capture_tick, tick_ms);
	tick_scheduled = true;
	LOG_MSG("CAPTURE: %u probe(s), %u verifier(s), %d Hz, emit=%s -> %s",
	        (unsigned)probes.size(), (unsigned)verifiers.size(),
	        rate_hz,
	        (emit_mode == EM_ON_CHANGE) ? "on_change" : "always",
	        capturefile.c_str());
}

void CAPTURE_STATE_Init() {
	control->AddSection_line("capture", &capture_init);
	MSG_Add("CAPTURE_CONFIGFILE_HELP",
		"Samples guest memory periodically and writes a CSV trace to disk.\n"
		"Lines (any order):\n"
		"  capturefile=PATH                       Output CSV path; empty = disabled.\n"
		"  rate_hz=N                              Sample frequency in Hz (default 60, max 100000).\n"
		"  emit_mode=always|on_change             always (default) emits every tick;\n"
		"                                         on_change emits only when at least one\n"
		"                                         probe value differs from the last emit.\n"
		"  LABEL:SEG:OFF:WIDTH                    Direct probe.\n"
		"  LABEL:ptr:SEG:POFF:DELTA:WIDTH         Indirect near: read 16-bit pointer\n"
		"                                         at SEG:POFF, sample at\n"
		"                                         SEG:(pointer+DELTA).\n"
		"  LABEL:farptr:SEG:POFF:DELTA:WIDTH      Indirect far: read 32-bit far ptr\n"
		"                                         (LE: offset then segment) at\n"
		"                                         SEG:POFF, sample at\n"
		"                                         resolved_seg:(resolved_off+DELTA).\n"
		"  LABEL:reg:NAME                         Sample a CPU register.  NAME =\n"
		"                                         cs ds es fs gs ss | ip ax bx cx dx\n"
		"                                         si di bp sp.  Useful for catching\n"
		"                                         the call site of an event of\n"
		"                                         interest (CS:IP at PIC tick time\n"
		"                                         points at the instruction the CPU\n"
		"                                         was about to execute).\n"
		"  verify=SEG:OFF:ASCII                   Positive sentinel.  Row is dropped\n"
		"                                         if memory at SEG:OFF does not\n"
		"                                         match ASCII.\n"
		"  denyif=SEG:OFF:ASCII                   Negative sentinel.  Row is dropped\n"
		"                                         if memory at SEG:OFF DOES match\n"
		"                                         ASCII.  Multiple verify/denyif\n"
		"                                         lines are AND'ed.  Use to gate\n"
		"                                         `ds`-symbolic probes when a single\n"
		"                                         positive match isn't specific\n"
		"                                         enough (e.g. distinguish two\n"
		"                                         Borland binaries from each other).\n"
		"  SEG may be a 4-hex-digit literal or 'ds' (live DS register).\n"
		"  All numeric fields are hex without 0x prefix.\n"
		"  WIDTH = byte | word | dword.\n"
		"  Whitespace around ':' is allowed.  '#' starts a comment.\n"
		"Example:\n"
		"  capturefile=game.csv\n"
		"  rate_hz=60\n"
		"  score        : 0040 : 0080 : dword\n"
		"  city0_farmer : ptr : ds : 89D2 : 15 : byte\n"
	);
}
