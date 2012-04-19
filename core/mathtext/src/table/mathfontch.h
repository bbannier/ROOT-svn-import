static const unsigned long nfont_change = 8;
static const char *
font_change_control_sequence[nfont_change] = {
	"\\mathbb", "\\mathbf", "\\mathfr", "\\mathit", "\\mathrm",
	"\\mathscr", "\\mathsf", "\\mathtt"
};
static const unsigned int font_change_family[nfont_change] = {
	math_symbol_t::FAMILY_MATH_BLACKBOARD_BOLD,
	math_symbol_t::FAMILY_BOLD,
	math_symbol_t::FAMILY_MATH_FRAKTUR_REGULAR,
	math_symbol_t::FAMILY_ITALIC,
	math_symbol_t::FAMILY_REGULAR,
	math_symbol_t::FAMILY_MATH_SCRIPT_ITALIC,
	math_symbol_t::FAMILY_MATH_SANS_SERIF_REGULAR,
	math_symbol_t::FAMILY_MATH_MONOSPACE
};
