#ident	"@(#)graf:src/whatis.d/sin.w	1.3"





sin [-_o_p_t_i_o_n] [_v_e_c_t_o_r(s)]; sin

Output is the sin for each element of the input _v_e_c_t_o_r(_s).
Input is assumed to be in radians.  If no _v_e_c_t_o_r is given,
the standard input is assumed.

Option:   c_n  _n is the number of output elements per line.

Example:   sin -c3 A
outputs the sin of each element of A, three per line.
