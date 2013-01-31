// Tralics, a LaTeX to XML translator.
// Copyright INRIA/apics/marelle (Jose Grimm) 2002-2011
// This file contains the array part of Tralics.

// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use, 
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info". 
// (See the file COPYING in the main directory for details)


#include "tralics.h"
const char* txarray_rcsid =
  "$Id: txarray.C,v 2.42 2011/06/28 08:15:25 grimm Exp $";

static NewArray new_array_object; 
static char char_for_error ='c';

void Parser::boot_NAO()
{
  new_array_object.boot(this);
}

void NewArray::boot(Parser*Q)
{
  P = Q;
  for(uint i=0; i<nb_newcolumn ;i++) {
    nct_bool[i] = false;
    nct_tok[i] = P->hash_table.relax_token;
  }
  nct_size = 0;
}

// This informs the system that char c is defined to be the token t.
void NewArray::add_a_type(uchar c, Token t)
{
  if(c==0) return;
  if(c&& nct_bool[c]== false)
    nct_size++;
  nct_bool[c] = true;
  nct_tok[c] = t;
}

// This informs the system that char c is now undefined.
void NewArray::remove_a_type(uchar c)
{
  if(nct_bool[c]) nct_size--;
  nct_bool[c] = false;
}

// Implements \newcolumntype{C}[opt]{body}
// as \newcommand\cmd[opt]{body}
// Here \cmd is some internal name, stored in nct_tok[C]
void Parser::T_newcolumn_type()
{
  TokenList L = mac_arg();
  uint c = 0; 
  if(L.empty()) parse_error("Empty argument to \\newcolumntype");
  else if(L.size() != 1)
    parse_error("More than one token in argument to \\newcolumntype");
  else {
    if(L.front().is_a_char())
      c = L.front().char_val().get_value();
    if(!(c>0 && c<nb_newcolumn)) {
      parse_error("Argument to \\newcolumntype is not a 7bit character");
      c= 0;
    }
  }
  Buffer& B = hash_table.my_buffer();
  B.reset();
  B.push_back("newcolumtype@");
  B.push_back(uchar(c)); // special hack if c=0 !
  cur_tok = hash_table.locate(B);
  new_array_object.add_a_type(c,cur_tok);
  back_input();
  get_new_command(rd_always,false); // definition is local
}

// Assume that T is the token defined and stored by the procedures above
// This returns -1 in case something strange happened (like redefinition)
// returns the number of arguments of the function otherwise.
int Parser::nct_aux(Token T)
{
  cur_tok = T;
  look_at_mac();
  if(!cur_cmd_chr.is_user()) 
    return -1; // bad
  Macro& X = mac_table.get_macro(cur_cmd_chr.get_chr());
  if(!(X.get_type() == dt_empty || X.get_type()==dt_normal))
    return -1;
  return X.get_nbargs();
}

// This piece of code expands all the \newcolumntype commands
// There is a special case: nct_size=0,  case where nothing is 
// defined, and nothing has to be done. Otherwise, we loop over all
// characters. If there is one substitution, we try again.
// If for some reason, the character is invalid, we remove it from the table.
void token_ns::expand_nct(TokenList&L)
{
  bool action = true;
  if(the_parser.tracing_commands())
    the_log << lg_start << "array preamble at start: " << L << "\n";
  if(!new_array_object.has_a_nct()) return;
  int max_iter = max_newcolumn_loops;
  while(action) {
    action = false;
    max_iter--;
    if(max_iter<=0) {
      the_parser.parse_error("array preamble expansion: Infinite loop?");
      return;
    }
    for(int i=1;i<int(nb_newcolumn);i++) {
      uchar c = i;
      if(!new_array_object.nct_exists(c)) continue;
      Token T = new_array_object.nct_token(c);
      int n = the_parser.nct_aux(T);
      if(n==-1) {
	new_array_object.remove_a_type(c);
	continue;
      }
      if(expand_nct(L,T,n,c,max_iter)) {
	action = true;
	if(the_parser.tracing_commands())
	  the_log << "array preamble after " << c << ": " << L << "\n";
      }
    }
  }
}

// Here we have to find the character c, and expand the token T.
// only top-level characters are considered. Active chars are allowed.
// MX is decreased. Job aborted if it becomes negative.
bool token_ns::expand_nct(TokenList&L,Token T, int n, uchar c, int& MX)
{ 
  TokenList res;
  bool result = false;
  TokenList Table[10]; // arguments of the commands 
  while(!L.empty()) {
    Token t = L.front(); 
    L.pop_front();
    if(t.is_a_left_brace()) {  // get a whole block
      L.push_front(t); // re-enter the brace in the list
      token_ns::fast_get_block(L,res);
      continue;
    }
    if(t.cmd_val() == 10)
      continue; // ignore spaces.
    if(!(t.is_a_char() && t.char_val()==c)) { // just copy
      res.push_back(t);
      continue;
    }
    result = true; // We found something
    MX--;
    if(MX<0) return true;
    for(int k=0;k<n;k++) 
      Table[k+1] = get_a_param(L,false);
    TokenList W = the_parser.special_expand(Table);
    L.splice(L.begin(),W);
  }
  L.splice(L.end(),res);
  return result;
}

// Prints a (ch_class, ch_num) pair for debug.
String NewArray::dump_slot()
{
  switch(ch_class) {
  case chc_cell: 
    switch(ch_num) {
    case chn_c: return "c ";
    case chn_l: return "l ";
    case chn_r: return "r ";
    case chn_m: return "m{} ";
    case chn_p: return "p{} ";
    case chn_b: return "b{} ";
    default:
      return "impossible!";
    }
  case chc_bar: return ch_num==0 ? "| " : "!{} ";
  case chc_v: return "<{} ";
  case chc_u: return ">{} ";
  case chc_start: return "impossible!";
  case chc_inter: return "@{} ";
  case chc_pre_bar: return "!";
  case chc_pre_inter: return "@";
  case chc_pre_v: return "<";
  case chc_pre_u: return ">";
  case chc_pre_cell:
    if(ch_num==chn_m) return "m";
    if(ch_num==chn_p) return "p";
    if(ch_num==chn_b) return "b";
    return "impossible!";
  default:
    return "impossible!";
  }
}

// This sets (ch_class,ch_num) according to last_ch_class  (case where
// a brace-enclosed expression is expected) or the next token 
// (in current_token, or the first element of current_list)
void NewArray::test_pach()
{
  if(last_ch_class==chc_pre_bar) { ch_class=chc_bar; ch_num = chn_l; return; } 
  if(last_ch_class==chc_pre_inter) { ch_class= chc_inter; return; }
  if(last_ch_class==chc_pre_v) { ch_class= chc_v; return; }
  if(last_ch_class==chc_pre_u) { ch_class= chc_u; return; }
  ch_class = chc_cell; // we should have a cell here.
  if(last_ch_class==chc_pre_cell) return; // was ok
  if(have_token_list) {
    if(current_list.empty()) current_token = Token();
    else 
      current_token = current_list.front();
  }
  uint c =0;
  if(current_token.cmd_val()==11 || current_token.cmd_val()==12
     || current_token.cmd_val()==13)
    c = current_token.char_val().get_value();
  if(c == 'c') 
    { cur_h_pos = np_c_center; ch_num = chn_c; return;  }
  if(c== 'l') 
    { cur_h_pos = np_c_left; ch_num = chn_l; return;  }
  if(c=='r') 
    { cur_h_pos = np_c_right; ch_num = chn_r; return;  }
  ch_num = chn_c; // in case of error
  // These want an argument
  char_for_error = c;
  if(c =='|') { ch_class = chc_bar; return; }
  if(c =='!') { ch_class = chc_pre_bar; return; }
  if(c =='@') { ch_class = chc_pre_inter; return; }
  if(c =='<') { ch_class = chc_pre_v; return; }
  if(c =='>') { ch_class = chc_pre_u; return; }
  ch_class = chc_pre_cell;
  if(c=='m') { ch_num = chn_m; return; }
  if(c=='p') { ch_num = chn_p; return; }
  if(c=='b') { ch_num = chn_b; return; }
  ch_class = chc_cell;
  the_parser.parse_error("Wrong character in array preamble ",
			 current_token, "", "Wrong char");
}

// Big function that handles the array preamble. We first read and
// expand the preamble, then loop over all characters, then check
// that it ends correctly. If main_fct is false, we are in a multicolumn.
// In this case, the array preamble should specify only material
// for one cell. We read the content C of the cell, and push back
// the {UCV} list (where U and V the optional <u> and <v> parts.
// braces are added because cells are assumed to be typeset in a group.
void NewArray::run(Xid ID, bool main_fct)
{
  id = ID;
  if(!main_fct) {
    Istring x = P->nT_next_arg();
    id.add_attribute(the_names[np_cols], x);
  }
  preamble = P->mac_arg();
  token_ns::expand_star(preamble);
  token_ns::expand_nct(preamble);
  if(main_fct) AI = &P->the_stack.new_array_info(id);
  u_list.clear();
  v_list.clear();
  ac_cell_no = 0;
  last_ch_class = chc_start;
  first_bar = true;
  bool seen = false;
  cur_h_pos = np_c_center;
  if(P->tracing_commands()) the_log << "array preamble parse: ";
  for(;;) {
    if(preamble.empty()) break;
    if(ac_next()) continue;
    test_pach();
    if(P->tracing_commands()) the_log << dump_slot();
    switch(ch_class) {
    case chc_cell:
      if(main_fct) ac_maybe_finish();  // finish previous
      else ac_maybe_finish_multi(seen);
      attribs.push_back(np_halign,cur_h_pos,false);  // set attributes
      cur_h_pos = np_c_center; // add a default
      current_list.clear();
      break;
    case chc_bar: 
      if(ch_num==chn_c) {
	if(last_ch_class==chc_start)
	  attribs.push_back(np_leftborder,np_true);
	else if(last_ch_class==chc_bar) {}
	else 
	  attribs.push_back(np_rightborder,np_true,false);
      }
      current_list.clear();
      break;
    case chc_v:  // got a <v>list, add it
      if(!have_token_list)
	v_list.push_front(current_token);
      else 
	v_list.splice(v_list.begin(),current_list);
      break;
    case chc_u: // got a <u>list, add it.
      if(!have_token_list)
	u_list.push_front(current_token);
      else
	u_list.splice(u_list.begin(),current_list);
      break;
    case chc_start: // preamble empty
      break;
    case chc_inter:
      current_list.clear();
      break;
    case chc_pre_bar:
      if(last_ch_class==chc_cell) break;
      else if(last_ch_class==chc_bar) break;
      else if(last_ch_class==chc_v) break;
      else if(last_ch_class==chc_u)
	P->parse_error("illegal ! or @");
      break;
    case chc_pre_inter:
      if(last_ch_class==chc_u)
	P->parse_error("illegal ! or @");
      break;
    case chc_pre_v:
      if(last_ch_class ==chc_cell) break;
      if(last_ch_class==chc_v) break;
      P->parse_error("unexpected less-than sign");
      ch_class = chc_pre_bar;
      break;
    case chc_pre_u:
    case chc_pre_cell:
      if(main_fct) ac_maybe_finish(); else ac_maybe_finish_multi(seen);
      break;
    }
    last_ch_class = ch_class;
  }
  switch(last_ch_class) {
  case chc_cell:
  case chc_bar:
  case chc_v:
  case chc_u:
  case chc_start:
  case chc_inter: break;
  default: {
    static Buffer B;
    B.reset();
    B << "Array preamble: argument missing for " << char_for_error;
    P->parse_error(B.c_str());
  }
  }
  if(P->tracing_commands()) the_log << "\n";
  if(main_fct) {
    ac_maybe_finish();
    return;
  }  else ac_maybe_finish_multi(seen);
  // Case of \multicolumn, 
  id.add_attribute(attribs);
  attribs.reset();
  TokenList cell = P->mac_arg();
  cell.splice(cell.begin(),u_list);
  cell.splice(cell.end(),v_list);
  u_list.clear();
  v_list.clear();
  P->brace_me(cell);
  P->back_input(cell);

}

// This is called when 
// case 1 we see a >
// case 2 we see m, p or b : 
// case 3 we see m{} p{} b{} or clr  : more will be done later
// in case m{}, this will be called twice (2 and 3) but nothing special happens
void NewArray::ac_maybe_finish()
{
  if(last_ch_class== chc_bar &&first_bar) {
    first_bar = false;
    return;
  }
  if(last_ch_class== chc_cell ||
     last_ch_class== chc_v ||
     last_ch_class== chc_bar) {
    AI->add_uv(u_list,v_list,attribs);
    attribs.reset();
    u_list.clear();
    v_list.clear();
    ac_cell_no ++;
  }
}

// This is called in the case of multicolumn
void NewArray::ac_maybe_finish_multi(bool& seen)
{
  if(last_ch_class== chc_bar &&first_bar) {
    first_bar = false;
    return;
  }
  if(last_ch_class== chc_cell ||
     last_ch_class== chc_v ||
     last_ch_class== chc_bar) {
    if(seen) P->parse_error("multi specs in \\multicolumn");
    seen = true;
  }
}

// Return true if this is a space.
// otherwise get the next token in current_token
// if it is a brace, get the list in current_list
bool NewArray::ac_next()
{
  current_token = preamble.front();
  if(current_token.cmd_val()==10) {
    preamble.pop_front();
    return true;
  }
  if(current_token==the_parser.hash_table.verb_token) {
    preamble.pop_front();
    return true;
  }
  if(current_token.is_a_left_brace()) {
    current_list = token_ns::fast_get_block(preamble);
    token_ns::remove_ext_braces(current_list);
    have_token_list = true;
  } else {
    have_token_list = false;
    preamble.pop_front();
  }
  return false;
}


// This starts or ends a cell, or does both
void Stack::push_pop_cell(int dir)
{
  if(dir != push_only) { pop(np_cell); add_nl(); }
  if(dir != pop_only) { push1(np_cell); }
}

// Start a row. If a=-1, it is the first row
// If a>0 it is a dimension to put between rows.
void Parser::start_a_row(int a)
{
  Xid prev_row(2); // dummy id as default value
  {
    Xmlp V = the_stack.top_stack()->last_addr();
    if(V) prev_row = V->get_id();
  }
  bool initial_hline=false;
  if(a>0) 
    prev_row.add_attribute(the_names[np_spaceafter],Istring(a));
  for(;;) {
    remove_initial_space_and_back_input();
    symcodes S = cur_cmd_chr.get_cmd();
    if(S==hline_cmd) {
      get_token();
      if(cur_cmd_chr.get_chr()==zero_code) {
	if(a<0) 
	  initial_hline=true;
	else
	  prev_row.add_attribute(np_bottomborder,np_true);
	continue;
      } else {
	T_cline();
	continue;
      }
    }
    if(S==end_cmd) return;
    the_stack.push1(np_row);
    if(initial_hline)
      the_stack.cur_xid().add_attribute(np_topborder,np_true);
    start_a_cell(false);
    return;
  }
}

// If started is true, we have already started one
void Parser::start_a_cell(bool started)
{
  if(!started) {
    the_stack.push_pop_cell(push_only);
    push_level(bt_cell);
  }
  remove_initial_space_and_back_input();
  symcodes S = cur_cmd_chr.get_cmd();
  if(S==unimp_cmd && cur_cmd_chr.get_chr()==omit_code) {
    get_token();
    the_stack.mark_omit_cell();
  } else if(S==end_cmd) { // has to be \end{tabular}
    the_stack.mark_omit_cell();
  } else if(S==multicolumn_cmd) {
    the_stack.mark_omit_cell();
    get_token();
    unprocessed_xml.remove_last_space();
    Xid cid = the_stack.get_top_id();
    new_array_object.run(cid,false);
  } else {
    TokenList L= the_stack.get_u_or_v(true);
    back_input(L);
  }
}

void Parser::finish_a_cell(Token T, Istring a)
{
  flush_buffer();
  the_stack.remove_last_space();
  if(!a.null()) {
    back_input(hash_table.space_token);
    TokenList L = token_ns::string_to_list(a);
    back_input(L);
    T = hash_table.crwithargs_token;
  }
  back_input(T);
  if(the_stack.is_omit_cell())
    return;
  TokenList L = the_stack.get_u_or_v(false);
  back_input(L);
}

// This is \begin{tabular}. Should be in an environment (not necessarily)
void Parser::T_start_tabular (subtypes c)
{
  Istring x = the_names[c==zero_code ? np_tabular : np_tabular_star];
  if(the_stack.is_float())
    leave_v_mode();
  else if(the_stack.is_frame(np_fbox)) {}
  else
    leave_h_mode();
  hash_table.eval_let_local("par","@empty");
  the_stack.push1(x,np_table);
  the_stack.add_att_to_last(np_rend,np_inline);
  Xid id = the_stack.get_xid();
  if(c!=0) { // case of tabular*
    Token T = cur_tok;
    TokenList L = mac_arg();
    ScaledInt tab_width = dimen_from_list(T,L);
    if(!tab_width.null())
      id.add_attribute(the_names[np_tab_width],the_main->SH.find_scaled(tab_width));
    get_token();  // eat the relax
    if(!cur_cmd_chr.is_relax()) back_input();
  }
  {
    int pos = get_ctb_opt(false); // Lamport does not mention c, who cares
    if(pos) id.add_attribute(the_names[np_vpos], the_names[pos]);
  }
  new_array_object.run(id,true);
  the_stack.set_array_mode(); // Is the mode ok ?
  start_a_row(-1);
}



// \\cline{3-4} : put a hrule on cells 3 to 4.
void Parser::T_cline()
{
  Token T = cur_tok;
  TokenList arg = mac_arg();
  back_input(hash_table.space_token);
  back_input(arg);
  int a = scan_int(T);
  get_token(); // should be a dash
  int b = scan_int(T);
  b = b-a;
  if(a>0 && b>=0) {
    Xmlp R = the_stack.top_stack()->last_addr();
    if(R) {
      if(R->ok_to_span(a,b,false)) {
	R->ok_to_span(a,b,true);
	return;
      }
    }
    the_stack.add_border(a,b);
  }
}

// This adds a-1 empty cells, then b+1 cells with a top_border
void Stack::add_border(int a, int b)
{
  push1(np_row);
  push_pop_cell(push_only);
  for(int i=1;i<a;i++) 
    push_pop_cell(push_and_pop);
  for(int i=0;i<b;i++) {
    add_att_to_last(np_topborder,np_true);
    push_pop_cell(push_and_pop);
  }
  add_att_to_last(np_topborder,np_true);
  push_pop_cell(pop_only);
  pop(np_row);
}

// This is executed when we see \end{tabular}
// If true, the \end{tabular} is not executed.
// and we have to push back the `end{tabular}' tokens
bool Parser::false_end_tabular(const string& s)
{
  if(the_stack.is_frame(np_cell)) {
    TokenList L = token_ns::string_to_list(s,true);
    back_input(L);
    back_input(hash_table.end_token);
    finish_a_cell(hash_table.cr_token,Istring());
    return true;
  }
  return false;
}


// This is the code of \end{tabular}
void Parser::T_end_tabular(subtypes c) 
{
  the_stack.pop(c==0 ? np_tabular : np_tabular_star);
  the_stack.delete_table_atts();
  if(!the_stack.is_float())
    leave_v_mode();
}

// 
void Parser::T_endv()
{
  if(the_stack.is_frame(np_cell)) {
    flush_buffer();
    the_stack.finish_cell(-1);
    the_stack.push_pop_cell(pop_only);
    pop_level(false,bt_cell);
    start_a_cell(false);
  }
  else parse_error("Bad endv token");
}

// This is done when we see a \\.
void Parser::T_cr()
{
  flush_buffer();
  int a = 0;
  if(cur_cmd_chr.get_chr() == crwithargs_code)
    a = scan_int(cur_tok);
  if(!the_stack.is_frame(np_cell)) {
    parse_error("bad \\cr");
    return;
  }
  the_stack.finish_cell(0);
  the_stack.push_pop_cell(pop_only);
  pop_level(false,bt_cell);
  the_stack.pop(np_row);
  start_a_row(a);
}

// Case of \multispan; fully expandable
void Parser::multispan()
{
  Token omit = hash_table.locate("omit");
  Token span = hash_table.locate("span");
  int n = scan_int(cur_tok);
  back_input(omit);
  while(n>1) { back_input(span); back_input(omit); --n; }
}

