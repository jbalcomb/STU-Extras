#include <idc.idc>

static main(void)
{
  SetCharPrm(INF_ENTAB, 1);
  SetCharPrm(INF_INDENT, 0);
  SetCharPrm(INF_COMMENT, 40);
  SetCharPrm(INF_XREFNUM, 1);
  SetCharPrm(INF_SHOWPREF, 1);
  SetCharPrm(INF_BORDER, 6);
  SetCharPrm(INF_PREFSEG, 1);
  SetCharPrm(INF_PREFFLAG, 1);
  SetCharPrm(INF_CMTFLAG, 9);
  SetCharPrm(INF_ORG, 1);
  SetCharPrm(INF_ASSUME, 1);
  SetShortPrm(INF_BINPREF, 16);
  SetCharPrm(INF_ASMTYPE, 1);
  SetLongPrm(INF_MAXREF, 16);
  Message("Restored IDA55 clean export settings.\n");
}
