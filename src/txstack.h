// -*- C++ -*-
// $Id: txstack.h,v 2.29 2009/06/12 14:11:00 grimm Exp $
// TRALICS, copyright (C) INRIA/apics (Jose' Grimm) 2004, 2007,2008


// This software is governed by the CeCILL license under French law and
// abiding by the rules of distribution of free software.  You can  use, 
// modify and/ or redistribute the software under the terms of the CeCILL
// license as circulated by CEA, CNRS and INRIA at the following URL
// "http://www.cecill.info". 
// (See the file COPYING in the main directory for details)

// This file holds the stack and related stuff


// Array management
class ArrayInfo
{
  Xid id; // the id of the table
  int cell_no; // current cell number, first on row is zero
  vector<AttList> attribs; // positions attributes for each row
  vector<TokenList> u_table, v_table;
  int size,repeat_count, first_repeat; // for repeated patterns
 public:
  ArrayInfo(Xid a1) : id(a1), cell_no(0), size(0), repeat_count(0), first_repeat(0) {}
  bool its_me(Xid a) { return id==a; }
  int get_cell_no() const { return cell_no; }
  void set_cell_no(int k) { cell_no = k; }
  AttList get_cell_atts(int k);
  TokenList get_u_or_v(bool,int);
  void add_uv(TokenList& u, TokenList& v,AttList);
  int get_size() { return size; }
  void del_array_info();
};

// This is the stack
class Stack {
  class StackSlot{
    Xmlp obj;
    int line;
    Istring frame;
    mode md;
    Istring uid;
    bool omit_cell;
    void dump();
    void fulldump(int);
    StackSlot(Xmlp a, int b, Istring c, mode M, Istring u) 
      : obj(a), line(b), frame(c), md(M), uid(u), omit_cell(false) {}
    friend class Stack;
  };
  vector <StackSlot> Table;
 private:
  int last_xid;  // id of the last 
  Istring cur_lid;    // the id to be pushed on uids[]
  vector<AttList> attributes; // the main table of attributes
  vector<Xml*> enames; // the main table of element namesx2
  Buffer mybuffer; // a buffer
  vector<ArrayInfo> AI;// the attributes for the current TeX arrays
  mode cur_mode;// the mode to be pushed on modes[]
 public:
  Stack();
  Xmlp newline_xml;
  bool in_v_mode() const { return get_mode() == mode_v; }
  bool in_h_mode() const { return get_mode() == mode_h; }
  bool in_no_mode() const { return get_mode() == mode_none; }
  bool in_bib_mode() const { return get_mode() == mode_bib; }
  bool in_array_mode() const { return get_mode() == mode_array; }
  mode get_mode() const { return cur_mode; }
  void hack_for_hanl();
  Xid get_xid() { return last_xid; } 
  Xid cur_xid() { return top_stack()->get_id(); }
  Xmlp document_element() { return Table[0].obj; }
  Xmlp top_stack() { return Table.back().obj; }
  Xmlp elt_from_id(int n) { return enames[n]; }
  Xmlp get_father();
  void set_mode(mode x) { cur_mode = x; }
  void set_arg_mode() { cur_mode = mode_argument; }
  void set_bib_mode() { cur_mode = mode_bib; }
  void set_h_mode() { cur_mode = mode_h; }
  void set_v_mode() { cur_mode = mode_v; }
  void set_m_mode() { cur_mode = mode_math; }
  void set_no_mode() { cur_mode = mode_none; }
  void set_array_mode() { cur_mode = mode_array; }
  void implement_cit(string b1, Istring b2, string a, string c);
  void ipush(Istring, Xmlp);
  void trace_pop(bool);
  void trace_stack();
  const StackSlot& first_non_empty() const;
  void dump();
  Xid next_xid(Xml*);
  void init_all(string a);
  void add_new_anchor();
  void add_new_anchor_spec();
  void add_anchor(const string&);
  void push_pop_cell(int);
  void add_border(int a, int b);
  void add_att_to_last(Istring,Istring,bool);
  void add_att_to_last(Istring,Istring);
  void add_att_to_last(name_positions,name_positions);
  void add_att_to_last(name_positions,Istring);
  void add_att_to_cur(Istring A, Istring B);
  void add_att_to_cur(Istring A, Istring B,bool);
  AttList& get_att_list(int k) { return attributes[k]; }
  bool is_frame(name_positions) const;
  bool is_frame2(name_positions) const;
  Istring first_frame() const;
  void push(Istring a, Xmlp b);
  void pop (Istring a);
  void pop (name_positions a);
  void push_trace();
  Xid push_par(int);
  AttList& add_newid0(name_positions);
  void add_sp_to_p (int pid, int vid);

  void fonts0 (name_positions x);
  Xmlp fonts1 (name_positions x);
  void push1(Istring name,name_positions x);
  void push1(name_positions x);
  void end_module ();
  void pop_if_frame(Istring x);
  void para_aux(int x);
  ArrayInfo& new_array_info(Xid);
  void finish_cell(int w);
  void delete_table_atts();
  void T_ampersand();
  void T_hline ();
  void add_nl();
  Xmlp temporary();
  void add_last(Xmlp);
  void unbox(Xmlp);
  Xmlp remove_last();
  Xmlp xml2_space(Istring a, Xmlp c, Xmlp d);
  Xmlp xml2_space(Istring a, Istring, Istring, Xmlp c, Xmlp d);
  Xmlp xml2_space(Istring a, Istring, Xmlp c, Xmlp d);
  Xid get_top_id() { return top_stack()->get_id(); }
  void find_cid_rid_tid(Xid&cid,Xid&rid,Xid&tid);
  TokenList get_u_or_v(bool u_or_v);
  ArrayInfo* find_cell_props(Xid id);
  void check_font();
  void add_center_to_p();
  Xmlp get_cur_par();
  Xmlp push_hbox(Istring);
  Istring get_cur_id() const  { return cur_lid; } 
  void set_cur_id(Istring k) { cur_lid = k; } 
  void add_last_string (const Buffer& B);
  void remove_last_space();
  void mark_omit_cell();
  bool is_omit_cell() { return Table.back().omit_cell; }
  ArrayInfo* get_my_table(Xid&);
  AttList& last_att_list() { return get_xid().get_att(); }
  bool is_float();
};

