// See also Knuth (1986), p. 170.
static const int nvr = INT_MIN;
static const int spacing_table[64] = {
	// right ->  Ord    Op   Bin   Rel  Open Close Punct Inner
	/* Ord   */    0,    1,   -2,   -3,    0,    0,    0,   -1,
	/* Op    */    1,    1,  nvr,   -3,    0,    0,    0,   -1,
	/* Bin   */   -2,   -2,  nvr,  nvr,   -2,  nvr,  nvr,   -2,
	/* Rel   */   -3,   -3,  nvr,    0,   -2,  nvr,  nvr,   -2,
	/* Open  */    0,    0,  nvr,    0,    0,    0,    0,    0,
	/* Close */    0,    1,   -2,   -3,    0,    0,    0,   -1,
	/* Punct */   -1,   -1,  nvr,   -1,   -1,   -1,   -1,   -1,
	/* Inner */   -1,    1,   -2,   -3,   -1,    0,   -1,   -1
};
