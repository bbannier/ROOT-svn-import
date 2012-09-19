void tmathtexttest(void)
{
	TCanvas *c1 = new TCanvas("c1");
#if 1
	TMathText l;
	l.SetTextAlign(23);
	l.SetTextSize(0.07);
	l.DrawMathText(0.5, 1, "\\prod_{j\\ge0} \\left(\\sum_{k\\ge0} a_{jk}z^k\\right) = \\sum_{n\\ge0} z^n \\left(\\sum_{k_0,k_1,\\ldots\\ge0\\atop k_0+k_1+\\cdots=n} a_{0k_0}a_{1k_1} \\cdots \\right)");
	l.DrawMathText(0.5, 0.8, "W_{\\delta_1\\rho_1\\sigma_2}^{3\\beta} = U_{\\delta_1\\rho_1\\sigma_2}^{3\\beta} + {1\\over 8\\pi^2} \\int_{\\alpha_1}^{\\alpha_2} d\\alpha_2^\\prime \\left[ {U_{\\delta_1\\rho_1}^{2\\beta} - \\alpha_2^\\prime U_{\\rho_1\\sigma_2}^{1\\beta} \\over U_{\\rho_1\\sigma_2}^{0\\beta}} \\right]");
	l.DrawMathText(0.5, 0.6, "d\\Gamma = {1\\over 2m_A} \\left( \\prod_f {d^3p_f\\over (2\\pi)^3} {1\\over 2E_f} \\right) \\left| \\mathscr{M} \\left(m_A - \\left\\{p_f\\right\\} \\right) \\right|^2 (2\\pi)^4 \\delta^{(4)} \\left(p_A - \\sum p_f \\right)");
	l.DrawMathText(0.5, 0.425, "4\\mathrm{Re}\\left\\{{2\\over 1-\\Delta\\alpha} \\chi(s) \\left[ \\^{g}_\\nu^e \\^{g}_\\nu^f (1 + \\cos^2\\theta) + \\^{g}_a^e \\^{g}_a^f \\cos\\theta \\right] \\right\\}");
	l.DrawMathText(0.5, 0.33, "p(n) = {1\\over\\pi\\sqrt{2}} \\sum_{k = 1}^\\infty \\sqrt{k} A_k(n) {d\\over dn} {\\sinh \\left\\{ {\\pi\\over k} \\sqrt{2\\over 3} \\sqrt{n - {1\\over 24}} \\right\\} \\over \\sqrt{n - {1\\over 24}}}");
	l.DrawMathText(0.3, 0.15, "{(\\ell+1)C_{\\ell}^{TE} \\over 2\\pi}");
	l.DrawMathText(0.5, 0.1, "\\hbox{RHIC スピン物理 Нью-Йорк}");
#else
	TLatex l;
	l.SetTextAlign(23);
	l.SetTextSize(0.06);
	l.DrawLatex(0.5, 0.95, "#prod_{j#geq0} #left(#sum_{k#geq0} a_{jk}z^{k}#right) = #sum_{n#geq0} z^{n} #left(#sum_{k_{0},k_{1},#3dots#geq0} a_{0k_{0}}a_{1k_{1}} #3dots #right)");
	l.DrawLatex(0.5, 0.8, "W_{#delta_{1}#rho_{1}#sigma_{2}}^{3#beta} = U_{#delta_{1}#rho_{1}#sigma_{2}}^{3#beta} + #frac{1}{8#pi^{2}} #int_{#alpha_{1}}^{#alpha_{2}} d#alpha'_{2} #left[ #frac{U_{#delta_{1}#rho_{1}}^{2#beta} - #alpha'_{2} U_{#rho_{1}#sigma_{2}}^{1#beta}}{U_{#rho_{1}#sigma_{2}}^{0#beta}} #right]");
	l.DrawLatex(0.5, 0.6, "d#Gamma = #frac{1}{2m_{A}} #left( #prod_{f} #frac{d^{3}p_{f}}{(2#pi)^{3}} #frac{1}{2E_{f}} #right) #left| M #left(m_{A} - #left{p_{f}#right} #right) #right|^{2} (2#pi)^{4} #delta^{(4)} #left(p_{A} - #sum p_{f} #right)");
	l.DrawLatex(0.5, 0.425, "4 Re #left{ #frac{2}{1 - #Delta#alpha} #chi(s) #[]{#hat{g}_{#nu}^{e}#hat{g}_{#nu}^{f} (1 + cos^{2}#theta) + 2 #hat{g}_{a}^{e}#hat{g}_{a}^{f} cos#theta) } #right}");
	l.DrawLatex(0.5, 0.3, "p(n) = #frac{1}{#pi#sqrt{2}} #sum_{k = 1}^{#infty} #sqrt{k} A_{k}(n) #frac{d}{dn} #frac{sinh #left{ #frac{#pi}{k} #sqrt{#frac{2}{3}} #sqrt{n - #frac{1}{24}} #right}}{#sqrt{n - #frac{1}{24}}}");
#endif
	c1->Print("c1.png");
	c1->Print("c1.ps");
	c1->Print("c1.eps");
}
