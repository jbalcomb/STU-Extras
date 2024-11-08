
// DASM should show conditional move
#define CLAMP(_val_, _lo_, _hi_) { (((_val_) < (_lo_)) ? _lo_ : (((_val_) > (_hi_)) ? (_hi_ : (_val_))) }
