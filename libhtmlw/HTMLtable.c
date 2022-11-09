/* Author: Gilles Dauphin
 * Version 3.0.1 [Jan97]
 *
 * Copyright (C) 1996 - G.Dauphin, P.Dax
 * See the file "license.mMosaic" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES. 
 *
 * 3D table borders from : malber@easynet.fr [Apr 98]
 *
 * VMS version 3.0 by George Cook [Mar 98]
 *
 * Copyright (C) 1998, 1999, 2000, 2002, 2004, 2005, 2006, 2007
 * The VMS Mosaic Project
 */

#include "../config.h"

#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <stdlib.h>
#include <ctype.h>

#include "HTMLmiscdefs.h"
#include "HTMLparse.h"
#include "HTMLP.h"
#include "HTMLPutil.h"
#include "HTMLfont.h"
#define DEFAULT_TABLE_CELLSPACING 2
#define DEFAULT_TABLE_CELLPADDING 1

#if defined(MULTINET) && defined(__DECC) && (__VMS_VER >= 70000000)
#define strdup decc$strdup
#endif
extern char *strdup();

#ifndef DISABLE_TRACE
int tableTrace;
extern int reportBugs;
extern int htmlwTrace;
extern int refreshTrace;
static void TableDump(TableInfo *t);
#endif

extern DescRec *DescType;


static void FreeColList(ColumnList *col_list)
{
	if (col_list->cells)
		free(col_list->cells);
	free(col_list);
}

static void FreeRowlist(RowList *row_list)
{
	int i;

	for (i = 0; i < row_list->row_count; i++)
		free(row_list->cells_lines[i]);
	free(row_list->cells_lines);
	free(row_list);
}

void _FreeTableStruct(TableInfo *t)
{
	FreeRowlist(t->row_list);
	free(t->col_max_w);
	free(t->col_min_w);
	free(t->col_w);
	free(t->col_req_w);
	free(t->col_abs_w);
	if (t->col_info)
		free(t->col_info);
	free(t);
}

static AlignType get_halign(char *tag, char *type)
{
	AlignType align = ALIGN_NONE;
	char *val = ParseMarkTag(tag, type, "ALIGN");

	if (val) {
	    if (caseless_equal(val, "LEFT")) {
		align = HALIGN_LEFT;
	    } else if (caseless_equal(val, "CENTER") ||
		       caseless_equal(val, "MIDDLE")) {
		align = HALIGN_CENTER;
	    } else if (caseless_equal(val, "RIGHT")) {
		align = HALIGN_RIGHT;
	    }
	    free(val);
	}
	return align;
}

static AlignType get_valign(char *tag, char *type)
{
	AlignType align = ALIGN_NONE;
	char *val = ParseMarkTag(tag, type, "VALIGN");

	if (val) {
	    if (caseless_equal(val, "TOP")) {
		align = VALIGN_TOP;
	    } else if (caseless_equal(val, "CENTER") ||
		       caseless_equal(val, "MIDDLE")) {
		align = VALIGN_MIDDLE;
	    } else if (caseless_equal(val, "BOTTOM")) {
		align = VALIGN_BOTTOM;
	    }
	    free(val);
	}
	return align;
}

static void test_colinfo_col(MarkInfo *sm, int *nr_cols)
{
	int span = 1;
	char *val = ParseMarkTag(sm->start, MT_COL, "SPAN");

	if (val) {
	    span = atoi(val);
	    free(val);
	}
	*nr_cols += span;
}

static void handle_colinfo_col(MarkInfo *sm, ColElemInfo *info,
			       ColElemInfo *ginfo, int *nr_cols, int group)
{
	int pos = *nr_cols;
	int span = 1;
	int abs_w = 0;
	int rel_w = 0;
	int prop_w = 0;
	AlignType halign, valign;
	char *val;

	if (val = ParseMarkTag(sm->start, MT_COL, "SPAN")) {
	    span = atoi(val);
	    free(val);
	}
	if (val = ParseMarkTag(sm->start, MT_COL, "WIDTH")) {
	    abs_w = atoi(val);
	    if (abs_w < 0) 
	        abs_w = 0;
	    if (strchr(val, '%')) {
		rel_w = abs_w;
		abs_w = 0;
		if (rel_w > 100)
	  	    rel_w = 100;
	    } else if (strchr(val, '*')) {
		prop_w = abs_w;
		abs_w = 0;
	    }
	    free(val);
   	}
	halign = get_halign(sm->start, MT_COL);
	valign = get_valign(sm->start, MT_COL);

	/* Inherit */
	if (ginfo) {
	    if (abs_w == 0)
		abs_w = ginfo->abs_width;
	    if (rel_w == 0)
		rel_w = ginfo->rel_width;
	    if (prop_w == 0)
		prop_w = ginfo->prop_width;
	    if (halign == ALIGN_NONE)
		halign = ginfo->halign;
	    if (valign == ALIGN_NONE)
		valign = ginfo->valign;
	}

	while (--span >= 0) {
	    info[pos].abs_width = abs_w;
	    info[pos].rel_width = rel_w;
	    info[pos].prop_width = prop_w;
	    info[pos].halign = halign;
	    info[pos].valign = valign;
	    info[pos].group = group;
	    ++pos;
   	}
	*nr_cols = pos;
}

static struct mark_up *test_colinfo_group(MarkInfo *sm, int *nr_cols)
{
	char *val = ParseMarkTag(sm->start, MT_COLGROUP, "SPAN");
	int gspan = 1;
	int cspan = 0;

	if (val) {
	    gspan = atoi(val);
	    free(val);
	}

	do {
	    sm = sm->next;
	    while (sm && sm->is_end)
		sm = sm->next;
	    if (!sm || (sm->type == M_COLGROUP))
		break;
	    if (sm->type == M_COL)
		test_colinfo_col(sm, &cspan);
   	} while(1);

	if (!cspan)
	    cspan = gspan;
	*nr_cols += cspan;
	return sm;
}

static struct mark_up *handle_colinfo_group(MarkInfo *sm, ColElemInfo *info,
					    int *nr_cols, int group)
{
	int pos = *nr_cols;
	int cspan = *nr_cols;
	int abs_w = 0;
	int rel_w = 0;
	int prop_w = 0;
	int gspan = 1;
	AlignType halign, valign;
	char *val;
	ColElemInfo ginfo;

	memset(&ginfo, 0, sizeof(ColElemInfo));

	if (val = ParseMarkTag(sm->start, MT_COLGROUP, "SPAN")) {
	    gspan = atoi(val);
	    free(val);
	}
	if (val = ParseMarkTag(sm->start, MT_COLGROUP, "WIDTH")) {
	    abs_w = atoi(val);
	    if (abs_w < 0)
		abs_w = 0;
	    if (strchr(val, '%'))  {
		rel_w = abs_w;
		abs_w = 0;
		if (rel_w > 100)
	  	    rel_w = 100;
	    } else if (strchr(val, '*')) {
		prop_w = abs_w;
		abs_w = 0;
  	    }
	    free(val);
   	}
	ginfo.halign = halign = get_halign(sm->start, MT_COLGROUP);
	ginfo.valign = valign = get_valign(sm->start, MT_COLGROUP);

	ginfo.abs_width = abs_w;
	ginfo.rel_width = rel_w;
	ginfo.prop_width = prop_w;
	ginfo.group = group;

	do {
	    sm = sm->next;
	    while (sm && sm->is_end)
		sm = sm->next;
	    if (!sm || (sm->type == M_COLGROUP))
		break;
	    if (sm->type == M_COL)
		handle_colinfo_col(sm, info, &ginfo, &cspan, group);
   	} while(1);

	if (cspan > pos) {
	    pos = cspan;
	} else {
	    while (--gspan >= 0)
		info[pos++] = ginfo;
	}
	*nr_cols = pos;
	return sm;
}

#ifndef DISABLE_TRACE
static void show_colinfo(TableInfo *table)
{
	int i = -1;
	ColElemInfo *info = table->col_info;

	fprintf(stderr, "\n----- COL/COLGROUP -----\n");
	if (!info)
	    return;
	while (++i < table->num_col) {
	    fprintf(stderr, "\n[%d]abs_w %3d rel_w %3d%% group %d", i,
		    info[i].abs_width, info[i].rel_width, info[i].group);
	    if (info[i].prop_width)
		fprintf(stderr," prop_w %3d*", info[i].prop_width);
	}
	fprintf(stderr, "\n------------------------\n\n");
}
#endif

static void apply_colinfo(MarkInfo *start, TableInfo *table)
{
	MarkInfo *sm = start;
	ColElemInfo *Elem_info;
	CellStruct **T_lines;
	CellStruct *T_cells;
	int Row, Col, Nr_rows, Pos, Max_pos, Type;
	int Abs_w, Rel_w, W, Nr_abs, Nr_rel;
	int Nr_cols = 0;
	int group = 0;

	do {
	    switch(sm->type) {
		case M_COL:
		    test_colinfo_col(sm, &Nr_cols);
		    sm = sm->next;
		    break;
		case M_COLGROUP:
		    sm = test_colinfo_group(sm, &Nr_cols);
		    break;
		default:
		    sm = sm->next;
		    break;
  	    }
	    while (sm && sm->is_end)
		sm = sm->next;
	} while(sm);

	if (Nr_cols != table->num_col) {
#ifndef DISABLE_TRACE
	    if (tableTrace || reportBugs)
	        fprintf(stderr,
		        "<%s>\n\tTABLE cols %d vs. COL/COLGROUP cols %d\n",
	    	        table->tb_start_mark->start, table->num_col, Nr_cols);
#endif
	    return;
        }

	Elem_info = (ColElemInfo *)calloc(1, sizeof(ColElemInfo) * Nr_cols);
	Col = 0;
	sm = start;

	do {
	    switch(sm->type) {
		case M_COL:
		    handle_colinfo_col(sm, Elem_info, NULL, &Col, group);
		    sm = sm->next;
		    break;
		case M_COLGROUP:
		    sm = handle_colinfo_group(sm, Elem_info, &Col, group++);
		    break;
		default:
		    sm = sm->next;
		    break;
	    }
	    while (sm && sm->is_end)
		sm = sm->next;
	} while(sm);

	/*
	 * If all values are applied here, one could free the vector
	 * at the end of this routine.
	 */
	table->col_info = Elem_info;

	T_lines = table->row_list->cells_lines;
	Nr_rows = table->num_row;
	Nr_cols = table->num_col;
	Row = -1;
	while (++Row < Nr_rows) {
	    T_cells = T_lines[Row];
	    Col = -1;
	    while (++Col < Nr_cols) {
		Type = T_cells[Col].cell_type;
		if (Type != M_TABLE_DATA && Type != M_TABLE_HEADER)
		    continue;
		Pos = Col - 1;
		Max_pos = Col + T_cells[Col].colspan;

		Abs_w = Rel_w = 0;
		while (++Pos < Max_pos) {
		    Abs_w += Elem_info[Pos].abs_width;
		    Rel_w += Elem_info[Pos].rel_width;
		}
		if (Abs_w) {
		    T_cells[Col].absolute_width = Abs_w;
		} else if (Rel_w) {
		    T_cells[Col].relative_width = Rel_w;
		}
		if (Elem_info[Col].halign != ALIGN_NONE)
	  	    T_cells[Col].halignment = Elem_info[Col].halign;
		if (Elem_info[Col].valign != ALIGN_NONE)
	  	    T_cells[Col].valignment = Elem_info[Col].valign;
		T_cells[Col].group = Elem_info[Col].group;
		Col = Max_pos - 1;
	    }
	}
	Abs_w = Rel_w = Nr_abs = Nr_rel = 0;
	Col = -1;
	while (++Col < Nr_cols) {
	    W = Elem_info[Col].abs_width;
	    if (W)
		Nr_abs++;
	    Abs_w += W;
	    W = Elem_info[Col].rel_width;
	    if (W)
		Nr_rel++;
	    Rel_w += W;
	}
	if (Nr_abs == Nr_cols) {
	    table->absolute_width = Abs_w;
	} else if (Nr_rel == Nr_cols) {
	    if (Rel_w > 100) 
		Rel_w = 100;
	    table->relative_width = Rel_w;
	}
}

static void UpdateColList(ColumnList **col_list, int td_count,
			  MarkType m_cell_type, MarkInfo *td_start_mark,
			  MarkInfo *td_end_mark, int colspan, int rowspan,
			  AlignType align, AlignType valign,
			  int awidth, int rwidth, int height, int nowrap,
			  MarkInfo *tr_start_mark)
{
	ColumnList *cl = *col_list;
	CellStruct *cells;
	int cell_count, cur_cell_num, i, nspan, ns;

	if (!cl) {  /* Create one structure */
		cl = (ColumnList *) calloc(1, sizeof(ColumnList));
		/*** set by calloc
		cl->cell_count = 0;
		cl->cells = NULL;
		***/
		cl->max_row_span = 1;
	}
	cur_cell_num = cell_count = cl->cell_count;

	cell_count += colspan;
	if (!cl->cells) {	/* Because a SunOS bug : GD 17 Dec 96 */
		cells = (CellStruct *)malloc(sizeof(CellStruct) * cell_count);
	} else {
		cells = (CellStruct *)realloc(cl->cells,
					      sizeof(CellStruct) * cell_count);
	}
	cells[cur_cell_num].td_count = td_count;
	cells[cur_cell_num].colspan = colspan;
	cells[cur_cell_num].rowspan = rowspan;
	cells[cur_cell_num].back_cs = 0;
	cells[cur_cell_num].back_rs = 0;
	cells[cur_cell_num].td_start = td_start_mark;
	cells[cur_cell_num].td_end = td_end_mark;
	cells[cur_cell_num].cell_type = m_cell_type;
	cells[cur_cell_num].height = 0;
	cells[cur_cell_num].width = 0;
	cells[cur_cell_num].halignment = align;
	cells[cur_cell_num].valignment = valign;
	cells[cur_cell_num].relative_width = rwidth;
	cells[cur_cell_num].absolute_width = awidth;
	cells[cur_cell_num].req_height = height;
	cells[cur_cell_num].treq_height = 0;
	cells[cur_cell_num].nowrap = nowrap;
	cells[cur_cell_num].tr_start = tr_start_mark;
	cells[cur_cell_num].group = 0;

	nspan = colspan - 1;
	cur_cell_num++;
	ns = nspan;
	for (i = 0; i < nspan; i++) {
		cells[cur_cell_num].td_count = td_count;
		cells[cur_cell_num].colspan = ns;
		cells[cur_cell_num].back_cs = i + 1;
		cells[cur_cell_num].back_rs = 0;
		cells[cur_cell_num].rowspan = rowspan;
		cells[cur_cell_num].td_start = NULL;
		cells[cur_cell_num].td_end = NULL;
		cells[cur_cell_num].cell_type = M_TD_CELL_PAD;
		cells[cur_cell_num].height = 0;
		cells[cur_cell_num].width = 0;
		cells[cur_cell_num].halignment = ALIGN_NONE;
		cells[cur_cell_num].valignment = ALIGN_NONE;
		cells[cur_cell_num].relative_width = 0;
		cells[cur_cell_num].absolute_width = 0;
		cells[cur_cell_num].req_height = 0;
		cells[cur_cell_num].nowrap = 0;
		cells[cur_cell_num].line_bottom = 0;
		cur_cell_num++;
		ns--;
	}
	cl->cells = cells;
	cl->cell_count = cell_count;
	if (rowspan > cl->max_row_span)
		cl->max_row_span = rowspan;
	*col_list = cl;
}

static void AddPadAtEndColList(ColumnList **cl, int toadd)
{
	int i;
	ColumnList *coll = *cl;

	coll->cells = (CellStruct *)realloc(coll->cells,
			       sizeof(CellStruct) * (coll->cell_count + toadd));
	for (i = coll->cell_count; i < (coll->cell_count + toadd); i++)
		coll->cells[i].cell_type = M_TD_CELL_PROPAGATE;
	coll->cell_count += toadd;
}

static void AddFreeAtEndColList(ColumnList **cl, int toadd)
{
	int i;
	ColumnList *coll = *cl;

	coll->cells = (CellStruct *)realloc(coll->cells,
			       sizeof(CellStruct) * (coll->cell_count + toadd));
	for (i = coll->cell_count; i < (coll->cell_count + toadd); i++) {
		coll->cells[i].td_count = 0;
		coll->cells[i].colspan = 1;
		coll->cells[i].rowspan = 1;
		coll->cells[i].back_cs = 0;
		coll->cells[i].back_rs = 0;
		coll->cells[i].td_start = NULL;
		coll->cells[i].td_end = NULL;
		coll->cells[i].height = 0;
		coll->cells[i].width = 0;
		coll->cells[i].cell_type = M_TD_CELL_FREE;
		coll->cells[i].relative_width = 0;
		coll->cells[i].absolute_width = 0;
		coll->cells[i].req_height = 0;
		coll->cells[i].nowrap = 0;
		coll->cells[i].line_bottom = 0;
	}
	coll->cell_count += toadd;
}

/* Add # of cells at end of line */
static void AddPadAtEndRowList(RowList *rl, int toadd)
{
	int i, j, bcs;

	for (i = 0; i < rl->row_count; i++) {  /* Add more cells in each line */
		if (!rl->cells_lines[i]) {
			rl->cells_lines[i] = (CellStruct *)calloc(
					rl->max_cell_count_in_line + toadd,
					sizeof(CellStruct));
		} else {
			rl->cells_lines[i] = (CellStruct *)realloc(
					rl->cells_lines[i], sizeof(CellStruct) *
					(rl->max_cell_count_in_line + toadd));
		}
	}
	/* Add PAD */
	for (i = 0; i < rl->low_cur_line_num; i++) {
		bcs = rl->cells_lines[i]
		      [rl->max_cell_count_in_line - 1].back_cs + 1;
		for (j = rl->max_cell_count_in_line;
		     j < (rl->max_cell_count_in_line + toadd); j++) {
			rl->cells_lines[i][j].td_count = 0;
			rl->cells_lines[i][j].colspan = 1;
			rl->cells_lines[i][j].rowspan = 1;
			rl->cells_lines[i][j].back_cs = bcs++;
			rl->cells_lines[i][j].back_rs = 0;
			rl->cells_lines[i][j].td_start = NULL;
			rl->cells_lines[i][j].td_end = NULL;
			rl->cells_lines[i][j].height = 0;
			rl->cells_lines[i][j].width = 0;
			rl->cells_lines[i][j].cell_type = M_TD_CELL_PAD;
			rl->cells_lines[i][j].relative_width = 0;
			rl->cells_lines[i][j].absolute_width = 0;
			rl->cells_lines[i][j].req_height = 0;
			rl->cells_lines[i][j].nowrap = 0;
			rl->cells_lines[i][j].line_bottom = 0;
		}
	}
	/* Add FREE */
	for (i = rl->low_cur_line_num; i < rl->row_count; i++) {
		for (j = rl->max_cell_count_in_line;
		     j < (rl->max_cell_count_in_line + toadd); j++) {
			rl->cells_lines[i][j].td_count = 0;
			rl->cells_lines[i][j].colspan = 1;
			rl->cells_lines[i][j].rowspan = 1;
			rl->cells_lines[i][j].back_cs = 0;
			rl->cells_lines[i][j].back_rs = 0;
			rl->cells_lines[i][j].td_start = NULL;
			rl->cells_lines[i][j].td_end = NULL;
			rl->cells_lines[i][j].height = 0;
			rl->cells_lines[i][j].width = 0;
			rl->cells_lines[i][j].cell_type = M_TD_CELL_FREE;
			rl->cells_lines[i][j].relative_width = 0;
			rl->cells_lines[i][j].absolute_width = 0;
			rl->cells_lines[i][j].req_height = 0;
			rl->cells_lines[i][j].nowrap = 0;
			rl->cells_lines[i][j].line_bottom = 0;
		}
	}
	rl->max_cell_count_in_line += toadd;
}

static void AddFreeLineToRow(RowList *rl, int toadd)
{
	CellStruct *ncl;
	int i, j;

	if (!rl->cells_lines) {
		rl->cells_lines = (CellStruct **)malloc(sizeof(CellStruct *) *
						       (rl->row_count + toadd));
	} else {
		rl->cells_lines = (CellStruct **)realloc(rl->cells_lines,
				sizeof(CellStruct *) * (rl->row_count + toadd));
	}
 	for (j = 0; j < toadd; j++) {
		ncl = (CellStruct *)calloc(1, sizeof(CellStruct) *
					   rl->max_cell_count_in_line);
		for (i = 0; i < rl->max_cell_count_in_line; i++) {
			/** Zeroed by calloc
			ncl[i].td_count = 0;
			ncl[i].back_cs = 0;
			ncl[i].back_rs = 0;
			ncl[i].td_start = NULL;
			ncl[i].td_end = NULL;
			ncl[i].height = 0;
			ncl[i].width = 0;
			ncl[i].relative_width = 0;
			ncl[i].absolute_width = 0;
			ncl[i].req_height = 0;
			ncl[i].nowrap = 0;
			ncl[i].line_bottom = 0;
			**/
			ncl[i].colspan = 1;
			ncl[i].rowspan = 1;
			ncl[i].cell_type = M_TD_CELL_FREE;
		}
		rl->cells_lines[rl->row_count] = ncl;
		rl->row_count++;
	}
}

static void UpdateRowList(RowList **row_list, int tr_count, ColumnList **cl)
{
	RowList *rl = *row_list;
	CellStruct work_cell, ref_cell;
	CellStruct *rcl;
	CellStruct *this_line = NULL;
	int low_cur_line_num, next_low_cur_line_num;
	int free_cell_found, n_rl_free_cell;
	int i, j, jc, nr;
	int ncell_for_this_cl = (*cl)->cell_count;
	int nrow_for_this_cl = (*cl)->max_row_span;

	/* Create Row List */
	if (!rl) {
		free_cell_found = 0;
		if ((nrow_for_this_cl > 1) && (ncell_for_this_cl == 1)) {
			/* A horrible hack to handle HTML like:
			 *	<TABLE><TR><TD ROWSPAN=2></TD></TR>
			 *	       <TR><TD></TD></TR></TABLE>
			 * so that there is a free cell on first line
			 */
			AddFreeAtEndColList(cl, 1);
			ncell_for_this_cl++;
			free_cell_found = 1;
		}
		this_line = (*cl)->cells;
		rl = (RowList *)malloc(sizeof(RowList));
		rl->row_count = nrow_for_this_cl;
		rl->low_cur_line_num = 0;
		rl->max_cell_count_in_line = ncell_for_this_cl;
		rl->cells_lines = (CellStruct **)malloc(
					  rl->row_count * sizeof(CellStruct *));
		for (i = 0; i < rl->row_count; i++)  /* Create cell in rows */
			rl->cells_lines[i] = (CellStruct *)malloc(
					ncell_for_this_cl * sizeof(CellStruct));
		for (j = 0; j < ncell_for_this_cl; j++) {  /* Copy first row */
		    	rl->cells_lines[0][j] = this_line[j];
			rl->cells_lines[0][j].tr_count = tr_count;
		}
		/* Now fill info for next lines */
		for (i = 1; i < rl->row_count; i++) {
			for (j = 0; j < ncell_for_this_cl; j++) {
				ref_cell = rl->cells_lines[0][j];
				work_cell = ref_cell;
				work_cell.td_start = NULL;
				work_cell.td_end = NULL;
				if (ref_cell.rowspan > i ) {
					work_cell.rowspan -= i;
					work_cell.back_rs = i;
					work_cell.cell_type = M_TD_CELL_PAD;
				} else {
					work_cell.rowspan = 1;
					work_cell.colspan = 1;
					work_cell.back_rs = 0;
					work_cell.back_cs = 0;
					work_cell.cell_type = M_TD_CELL_FREE;
					if (!free_cell_found) {
						free_cell_found = 1;
						rl->low_cur_line_num = i;
					}
				}
				rl->cells_lines[i][j] = work_cell;
			}
		}
		if (!free_cell_found)
			rl->low_cur_line_num = i - 1;
		*row_list = rl;
		return;
	}
	/* The low_cur_line_num has an M_TD_CELL_FREE or is an empty line */
	low_cur_line_num = rl->low_cur_line_num;
	n_rl_free_cell = 0;
	rcl = rl->cells_lines[low_cur_line_num];
	/* Count the number of free cells */
	for (i = 0; i < rl->max_cell_count_in_line; i++) {
		if (rcl[i].cell_type == M_TD_CELL_FREE)
			n_rl_free_cell++;
	}
	if (n_rl_free_cell == 0) {	/* Add an empty line */
		AddFreeLineToRow(rl, 1);
		rl->low_cur_line_num = ++low_cur_line_num;
		/* n_rl_free_cell = rl->max_cell_count_in_line; */
		rcl = rl->cells_lines[low_cur_line_num];
		n_rl_free_cell = 0;
		for (i = 0; i < rl->max_cell_count_in_line; i++) {
			if (rcl[i].cell_type == M_TD_CELL_FREE)
				n_rl_free_cell++;
		}
	}
	if (ncell_for_this_cl < n_rl_free_cell)
		AddPadAtEndColList(cl, n_rl_free_cell - ncell_for_this_cl); 

	if (ncell_for_this_cl > n_rl_free_cell) {
#ifndef DISABLE_TRACE
		if (tableTrace || reportBugs) {
			fprintf(stderr,
				"Number of TD/TH or span count is bad.\n");
			fprintf(stderr,	"Adding %d columns to table.\n",
				ncell_for_this_cl - n_rl_free_cell);
		}
#endif
		AddPadAtEndRowList(rl, ncell_for_this_cl - n_rl_free_cell);
		/* From low_cur_line_num inclusive to row_count - 1,
		 * set as FREE */
	}
	this_line = (*cl)->cells;
	rcl = rl->cells_lines[low_cur_line_num];
	nrow_for_this_cl = (*cl)->max_row_span;

	/* If nrow_for_this_cl + low_cur_line_num > row_count, then
	 * extend the table, increase the number of lines, all set to FREE */
	if ((nrow_for_this_cl + low_cur_line_num) > rl->row_count)
		AddFreeLineToRow(rl,
			   nrow_for_this_cl + low_cur_line_num - rl->row_count);

	/* Now (*cl)->cell_count and n_rl_free_cell are equal
	 * and the number of lines in rl is sufficient */
	jc = 0;
	for (i = 0; i < rl->max_cell_count_in_line; i++) {
		if (rcl[i].cell_type == M_TD_CELL_FREE) {
			ref_cell = this_line[jc];
			if (ref_cell.cell_type == M_TD_CELL_PROPAGATE) {
				memset(&ref_cell, 0, sizeof(ref_cell));
				ref_cell.cell_type= M_TD_CELL_PROPAGATE;
				ref_cell.colspan = 1;
				ref_cell.rowspan = 1;
				rcl[i] = ref_cell;
				jc++;
				continue;
			}
			work_cell = ref_cell;
			/* Watch out for row spans when you add a row, set
			 * them to FREE except columns [i..i + colspan] where
			 * set PAD with rowspan */
			for (nr = 1; nr < ref_cell.rowspan; nr++) {
				work_cell.rowspan = ref_cell.rowspan - nr;
				work_cell.back_rs++;
				work_cell.td_start = NULL;
				work_cell.td_end = NULL;
				work_cell.cell_type = M_TD_CELL_PAD;
				rl->cells_lines[low_cur_line_num + nr][i] =
								      work_cell;
			}
			rcl[i] = ref_cell;
			jc++;
		}
	}
	/* Compute the new low_cur_line_num */
	next_low_cur_line_num = low_cur_line_num + 1;
	free_cell_found = 0;
	for (i = next_low_cur_line_num; i < rl->row_count; i++) {
		for (j = 0; j < rl->max_cell_count_in_line; j++) {
			if (rl->cells_lines[i][j].cell_type == M_TD_CELL_FREE) {
				free_cell_found = 1;
				rl->low_cur_line_num = i;
				break;
			}
		}
		if (free_cell_found)
			break;
	}
	*row_list = rl;
}

static TableInfo *FirstPassTable(HTMLWidget hw, MarkInfo *mptr,
				 PhotoComposeContext *pcc)
{
	TableInfo *t;
	TableInfo lt;
	MarkInfo *tb_start_mark;    /* Save the marker <TABLE> */
	MarkInfo *tb_end_mark = NULL;
	MarkInfo *start_other_mark; /* Is mark between TABLE and TR */
				    /* or CAPTION? */
	MarkInfo *end_other_mark, *caption_end_mark, *caption_start_mark;
	MarkInfo *tr_start_mark, *td_start_mark, *td_end_mark;
	MarkInfo *sm, *psm, *mark, *start_col_info, *end_col_info;
	MarkType m_cell_type = M_TD_CELL_PAD;
	ColumnList *col_list;
	RowList *row_list;
	AlignType tmp, align, row_align, valign, row_valign, save_align;
	char *val, *tptr;
	char *mt_cell_type = NULL;
	int tr_found, caption_found, end_caption_found;
	int awidth, rwidth, height,nowrap, i, j;
	int td_count = 0;
	int tr_count = 0;
	int tr_start_found = 0;
	int td_start_found = 0;
	int colspan = 0;
	int rowspan = 0;
	int table_nowrap = 0;

	/* mptr is on <TABLE> */
	tb_start_mark = mptr;
	sm = mptr->next;
	/* 'sm' points just after <TABLE> */

	memset(&lt, 0, sizeof(TableInfo));
	lt.frame = VOID;
	lt.rules = NONE;
	lt.align = pcc->div;
	lt.cellpadding = DEFAULT_TABLE_CELLPADDING;
	lt.cellspacing = DEFAULT_TABLE_CELLSPACING;

	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "BORDER")) {
		if (*tptr) {
			lt.outer_border = atoi(tptr);
			if (lt.outer_border)
				lt.borders = 1;
		} else {
			lt.outer_border = lt.borders = 1;
		}
		if (lt.borders) {
			lt.frame = BOX;
			lt.rules = ALL;
		}
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "FRAME")) {
		if (*tptr) {
			if (caseless_equal(tptr, "VOID")) {
				lt.frame = VOID;
				lt.outer_border = 0;
			} else if (caseless_equal(tptr, "ABOVE")) {
				lt.frame = ABOVE;
			} else if (caseless_equal(tptr, "BELOW")) {
				lt.frame = BELOW;
			} else if (caseless_equal(tptr, "HSIDES")) {
				lt.frame = HSIDES;
			} else if (caseless_equal(tptr, "VSIDES")) {
				lt.frame = VSIDES;
			} else if (caseless_equal(tptr, "LHS")) {
				lt.frame = LHS;
			} else if (caseless_equal(tptr, "RHS")) {
				lt.frame = RHS;
			} else if (caseless_equal(tptr, "BOX") ||
				   caseless_equal(tptr, "BORDER")) {
				lt.frame = BOX;
			}
			if (lt.frame != VOID)
				lt.borders = 1;
		}
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "RULES")) {
		if (*tptr) {
			if (caseless_equal(tptr, "NONE")) {
				lt.rules = NONE;
			} else if (caseless_equal(tptr, "GROUPS")) {
				lt.rules = GROUPS;
			} else if (caseless_equal(tptr, "ROWS")) {
				lt.rules = ROWS;
			} else if (caseless_equal(tptr, "COLS")) {
				lt.rules = COLS;
			} else if (caseless_equal(tptr, "ALL")) {
				lt.rules = ALL;
			} else {
				lt.rules = NONE;
			}
			if (lt.rules != NONE)
				lt.borders = 1;
		}
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "WIDTH")) {
		if (*tptr) {
			lt.absolute_width = atoi(tptr);
			if (strchr(tptr, '%')) {	/* Relative value */
				lt.relative_width = lt.absolute_width;
				lt.absolute_width = 0;
			}
		}
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "HEIGHT")) {
		if (*tptr && !strchr(tptr, '%'))
			lt.height = atoi(tptr);
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "ALIGN")) {
		if (caseless_equal(tptr, "LEFT")) {
			/* Use HALIGN_LEFT so only do it if explicitly asked */
			lt.align = HALIGN_LEFT;
		} else if (caseless_equal(tptr, "CENTER")) {
			lt.align = DIV_ALIGN_CENTER;
		} else if (caseless_equal(tptr, "RIGHT")) {
			/* Use HALIGN_RIGHT so only do it if explicitly asked */
			lt.align = HALIGN_RIGHT;
		}
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "CELLSPACING")) {
		if (*tptr) 
	 		lt.cellspacing = atoi(tptr);
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "CELLPADDING")) {
		if (*tptr) 
			lt.cellpadding = atoi(tptr);
		free(tptr);
	}
	if (tptr = ParseMarkTag(mptr->start, MT_TABLE, "NOWRAP")) {
		table_nowrap = 1;
		free(tptr);
	}

	/* Find the first TR or CAPTION */
	caption_found = tr_found = 0;
	start_other_mark = end_other_mark = caption_end_mark = NULL;
	while (sm) {
		/* Ignore scripts and style sheets */
		if ((sm->type == M_SCRIPT) || (sm->type == M_STYLE)) {
			/* Script and style contents removed by htmlparse,
			 * so only start and end tags remain. */
			sm = sm->next;
			continue;
		}
		if ((sm->type == M_CAPTION) && !sm->is_end) {
			lt.captionAlignment = VALIGN_TOP;
			if (tptr = ParseMarkTag(sm->start, MT_CAPTION,
			    			"ALIGN")) {
				if (caseless_equal(tptr, "BOTTOM")) {
					lt.captionAlignment = VALIGN_BOTTOM;
				} else if (caseless_equal(tptr, "LEFT")) {
					lt.captionAlignment = HALIGN_LEFT;
				} else if (caseless_equal(tptr, "CENTER")) {
					lt.captionAlignment = HALIGN_CENTER;
				} else if (caseless_equal(tptr, "RIGHT")) {
					lt.captionAlignment = HALIGN_RIGHT;
				}
				free(tptr);
			}
			lt.captionIsLegend = 0;
			if (tptr = ParseMarkTag(sm->start, MT_CAPTION,
			    			"LEGEND")) {
				if (*tptr)
					lt.captionIsLegend = atoi(tptr);
				free(tptr);
			}
			caption_found = 1;
			caption_start_mark = sm;
			break;
		}
		if ((sm->type == M_TABLE_ROW || sm->type == M_TABLE_DATA ||
		     sm->type == M_TABLE_HEADER) && !sm->is_end) {
			tr_found = 1;
			tr_start_mark = sm;
			break;
		}
		if (!sm->is_white_text && !start_other_mark) {
			start_other_mark = sm;
			lt.other_before_caption = 1;
		}
		if (start_other_mark)
			end_other_mark = sm;

		/* Check for table end */
		if ((sm->type == M_TABLE) && sm->is_end)
			break;
		/* Check for another table start */
		if (sm->type == M_TABLE) {
			if (!start_other_mark)
				start_other_mark = sm;
			break;
		}
		sm = sm->next;
	}
	lt.start_other_mark = start_other_mark;
	lt.end_other_mark = end_other_mark;
	if (start_other_mark && !caption_found)
		lt.other_before_caption = 0;

	if (!caption_found && !tr_found && !start_other_mark) {
#ifndef DISABLE_TRACE
		if (tableTrace || reportBugs)
			fprintf(stderr, "Nothing in table.\n");
#endif
		return NULL;
	}
	if (caption_found) {  /* Find CAPTION end */
		end_caption_found = 0;
		caption_end_mark = caption_start_mark;
		while (caption_end_mark) {
			if (((caption_end_mark->type == M_CAPTION) &&
			     caption_end_mark->is_end) ||
			    (caption_end_mark->type == M_TABLE_ROW) ||
			    (caption_end_mark->type == M_TABLE_DATA) ||
			    (caption_end_mark->type == M_TABLE_HEADER) ||
			    ((caption_end_mark->type == M_TABLE) &&
			     caption_end_mark->is_end)) {
				end_caption_found = 1;
				break;
			}
			psm = caption_end_mark;
			caption_end_mark = caption_end_mark->next;
		}
		if (!end_caption_found) {
#ifndef DISABLE_TRACE
			if (tableTrace || reportBugs)
				fprintf(stderr, "</CAPTION> not found\n");
#endif
			return NULL;
		}
		if ((caption_end_mark->type == M_CAPTION) &&
		    caption_end_mark->is_end) {
			sm = caption_end_mark->next;
		} else {
			sm = GetMarkRec();
			sm->type = M_CAPTION;
			sm->is_end = 1;
			/* Link it into markup list */
			sm->next = caption_end_mark;
			psm->next = sm;
			sm = caption_end_mark;
			caption_end_mark = psm->next;
		}
		lt.caption_start_mark = caption_start_mark;
		lt.caption_end_mark = caption_end_mark;
		if (!tr_found && !start_other_mark) {
			while (sm) {
				if ((sm->type == M_TABLE_ROW ||
				     sm->type == M_TABLE_DATA ||
				     sm->type == M_TABLE_HEADER) &&
				    !sm->is_end) {
					tr_found = 1;
					tr_start_mark = sm;
					break;
				}
				if ((sm->type == M_TABLE) && sm->is_end)
					break;
				if (!sm->is_white_text && !start_other_mark)
					start_other_mark = sm;
				if (start_other_mark)
					end_other_mark = sm;
				sm = sm->next;
			}
			if (!tr_found &&
			    (!start_other_mark || lt.other_before_caption)) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
					     "Nothing but caption in table.\n");
#endif
				return NULL;
			}
			lt.start_other_mark = start_other_mark;
			lt.end_other_mark = end_other_mark;
			lt.other_before_caption = 0;
		}
	}

	/* Now find the first <TR> */
	if (!tr_found) {
		if (caption_end_mark) {
			tr_start_mark = caption_end_mark;
		} else {
			tr_start_mark = start_other_mark;
		}
		while (tr_start_mark) {
			if ((tr_start_mark->type == M_TABLE_ROW) &&
			    !tr_start_mark->is_end) {
				tr_found = 1;
				break;
			}
			/* If TABLE tag, fall thru */
			if (tr_start_mark->type == M_TABLE)
				break;
			tr_start_mark = tr_start_mark->next;
		}
		if (!tr_found &&
		    (!start_other_mark || lt.other_before_caption)) {
#ifndef DISABLE_TRACE
			if (tableTrace || reportBugs)
				fprintf(stderr, "Table has no rows.\n");
#endif
			return NULL;
		}
	}

	/* COL and COLGROUP */
	start_col_info = end_col_info = NULL;
	if (start_other_mark) {
		sm = start_other_mark;
		while (sm && !start_col_info) {
			if (sm->type == M_COL || sm->type == M_COLGROUP) {
				start_col_info = end_col_info = sm;
				if (sm == start_other_mark) {
					if (sm == end_other_mark) {
						start_other_mark = NULL;
					} else {
						start_other_mark = sm->next;
					}
				}
			}
			if (sm == end_other_mark)
				break;
			sm = sm->next;
		}
		if (start_col_info) {
			while (sm) {
				if (sm->type == M_COL ||
				    sm->type == M_COLGROUP) {
					end_col_info = sm;
					if (sm == start_other_mark) {
						if (sm == end_other_mark) {
						    start_other_mark = NULL;
						} else {
						    start_other_mark = sm->next;
						}
					}
				}						
				if (sm == end_other_mark)
					break;
				sm = sm->next;
			}
		}
		lt.start_other_mark = start_other_mark;
	}

	/* Create tr_start_mark if one not found, but found other stuff */
	if (!tr_found && start_other_mark) {
#ifndef DISABLE_TRACE
		if (tableTrace || reportBugs)
			fprintf(stderr, "Table has no <TR> or <TD>/<TH> tag\n");
#endif
		if (caption_end_mark) {
			psm = caption_end_mark;
		} else {
			psm = tb_start_mark;
		}
		tr_start_mark = GetMarkRec();
		tr_start_mark->type = M_TABLE_DATA;
		tr_start_mark->is_end = 0;
		tr_start_mark->start = GetMarkText("TD");
		/* Link it into markup list */
		tr_start_mark->next = start_other_mark;
		psm->next = tr_start_mark;
		lt.start_other_mark = NULL;
	}

	psm = NULL;
	sm = lt.first_tr_mark = tr_start_mark;
	td_start_mark = td_end_mark = tb_end_mark = NULL;
	td_count = tr_count = 0;
	row_list = NULL;
	col_list = NULL;
	tr_start_found = td_start_found = 0;
	align = row_align = ALIGN_NONE;
	row_valign = VALIGN_MIDDLE;

	while (sm) {
		/* Handle <TD> or <TH> or text outside table tags */
		if (((sm->type == M_TABLE_DATA) ||
		     (sm->type == M_TABLE_HEADER) ||
		     (!td_start_found && (sm->type == M_NONE) &&
		      !sm->is_white_text)) &&
		    !sm->is_end) {
			if (!tr_start_found) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
					     "A <TD>/<TH> is outside a <TR>\n");
#endif
				/* Let's play pretend */
				tr_start_found = 1;	/* Imagine a <TR> */
			}
			if (td_start_found) {  /* This is the end of previous */
				td_count++;
				td_end_mark = psm;
				UpdateColList(&col_list, td_count, m_cell_type,
					 td_start_mark, td_end_mark, colspan,
					 rowspan, align, valign, awidth, rwidth,
					 height, nowrap, tr_start_mark);
			} else if (sm->type == M_NONE) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
						"Table text without <TD>\n");
#endif
				/* Let's pretend by faking a <TD> */
				mark = GetMarkRec();
				mark->type = M_TABLE_DATA;
				mark->is_end = 0;
				mark->start = GetMarkText("TD");
				/* Link it into markup list */
				mark->next = sm;
				psm->next = mark;
				sm = mark;
			}
			if (sm->type == M_TABLE_DATA) {
				m_cell_type = M_TABLE_DATA;
				mt_cell_type = MT_TABLE_DATA;
			} else {
				m_cell_type = M_TABLE_HEADER;
				mt_cell_type = MT_TABLE_HEADER;
			}
			td_start_found = 1;
			td_start_mark = sm;

			colspan = rowspan = 1;
			val = ParseMarkTag(sm->start, mt_cell_type, "colspan");
			if (val) {
				if (*val)
					colspan = atoi(val);
				free(val);
			}
			if (colspan <= 0) 
				colspan = 1;
			val = ParseMarkTag(sm->start, mt_cell_type, "rowspan");
			if (val) {
				if (*val)
					rowspan = atoi(val);
				free(val);
			}
			if (rowspan <= 0) 
				rowspan = 1;

			align = get_halign(sm->start, mt_cell_type);
			if (align == ALIGN_NONE)
				align = row_align;
			valign = get_valign(sm->start, mt_cell_type);
			if (valign == ALIGN_NONE)
				valign = row_valign;

			rwidth = awidth = 0;
			/* Don't know how to handle width spanning columns */
			if (colspan < 2) {
				val = ParseMarkTag(sm->start, mt_cell_type,
						   "WIDTH");
			} else {
				val = NULL;
			}
			if (val) {
				if (*val) {
					awidth = atoi(val);
					if (strchr(val, '%')) { 
						/* Relative value */
						rwidth = awidth;
						awidth = 0;
					}
				}
				free(val);
			}
			height = 0;
			val = ParseMarkTag(sm->start, mt_cell_type, "HEIGHT");
			if (val) {
				if (*val)
					height = atoi(val);
				free(val);
			}
			val = ParseMarkTag(sm->start, mt_cell_type, "NOWRAP");
			if (val) {
				nowrap = 1;
				free(val);
			} else {
				nowrap = table_nowrap;
			}
			psm = sm;
			sm = sm->next;
			continue;
		}				/* <TH> or <TD> */

		/* Handle </TH> or </TD> */
		if (((sm->type == M_TABLE_DATA) || (sm->type == M_TABLE_HEADER))
		    && sm->is_end) {
			if (!tr_start_found) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
	                        	fprintf(stderr,
					       "</TD>/</TH> is outside <TR>\n");
#endif
				psm = sm;
				sm = sm->next;
                                continue;
                        }
			if (!td_start_found) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
	                                fprintf(stderr,
						"A </TD> is without a <TD>\n");
#endif
				psm = sm;
				sm = sm->next;
                                continue;
                        }
			td_count++;
			td_end_mark = sm;
			UpdateColList(&col_list, td_count, m_cell_type,
				  td_start_mark, td_end_mark, colspan, rowspan,
				  align, valign, awidth, rwidth, height, nowrap,
				  tr_start_mark);
			td_start_found = 0;
			psm = sm;
			sm = sm->next;
			continue;
		}

		/* Handle <TR> */
		if ((sm->type == M_TABLE_ROW) && !sm->is_end) {
			if (td_start_found) {
				td_count++;
				td_end_mark = psm;
				UpdateColList(&col_list, td_count, m_cell_type,
					 td_start_mark, td_end_mark, colspan,
					 rowspan, align, valign, awidth, rwidth,
					 height, nowrap, sm);
				td_start_found = 0;
			}
			if (tr_start_found) {
				tr_count++;
				if (col_list) {
					UpdateRowList(&row_list, tr_count, 
						      &col_list);
					FreeColList(col_list);
					col_list = NULL;
					td_count = 0;
#ifndef DISABLE_TRACE
				} else if (tableTrace || reportBugs) {
					fprintf(stderr,	"<TR> without <TD>\n");
#endif
				}
			}
			row_align = get_halign(sm->start, MT_TABLE_ROW);
			row_valign = get_valign(sm->start, MT_TABLE_ROW);
			if (row_valign == ALIGN_NONE)
				row_valign = VALIGN_MIDDLE;

			tr_start_found = 1;
			tr_start_mark = sm;
			psm = sm;
			sm = sm->next;
			continue;
		}

		/* Handle </TR> */
		if ((sm->type == M_TABLE_ROW) && sm->is_end) {
			if (!tr_start_found) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
						"A </TR> without <TR>\n");
#endif
				psm = sm;
				sm = sm->next;
				continue;
			}
			if (td_start_found) {
				td_count++;
				td_end_mark = psm;
				UpdateColList(&col_list, td_count, m_cell_type,
					 td_start_mark, td_end_mark, colspan,
					 rowspan, align, valign, awidth, rwidth,
					 height, nowrap, tr_start_mark);
				td_start_found = 0;
			}
			if (col_list) {
				tr_count++;
				UpdateRowList(&row_list, tr_count, &col_list);
				FreeColList(col_list);
				col_list = NULL;
				td_count = 0;
			} else {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
						"A </TR> without <TD>\n");
#endif
				/* Let's pretend some more by faking a <TD> */
				mark = GetMarkRec();
				mark->type = M_TABLE_DATA;
				mark->is_end = 0;
				mark->start = GetMarkText("TD");
				/* Link it into markup list */
				mark->next = tr_start_mark->next;
				tr_start_mark->next = mark;
				sm = mark;
				continue;
			}
			tr_start_found = 0;
			psm = sm;
			sm = sm->next;
			continue;
		}

		/* Handle </TABLE> */
		if ((sm->type == M_TABLE) && sm->is_end) {
			if (td_start_found) {
				td_count++;
				td_end_mark = psm;
				UpdateColList(&col_list, td_count, m_cell_type,
					 td_start_mark, td_end_mark, colspan,
					 rowspan, align, valign, awidth, rwidth,
					 height, nowrap, tr_start_mark);
				td_start_found = 0;
			}
			if (tr_start_found) {
				if (col_list) {
					tr_count++;
					UpdateRowList(&row_list, tr_count, 
						      &col_list);
					FreeColList(col_list);
					col_list = NULL;
					td_count = 0;
				} else {
#ifndef DISABLE_TRACE
					if (tableTrace || reportBugs)
					    fprintf(stderr,
					     "<TR> without <TD> at </TABLE>\n");
#endif
					/* An yet another fake <TD> */
					mark = GetMarkRec();
					mark->type = M_TABLE_DATA;
					mark->is_end = 0;
					mark->start = GetMarkText("TD");
					/* Link it into markup list */
					mark->next = tr_start_mark->next;
					tr_start_mark->next = mark;
					sm = mark;
					continue;
				}
			}
			if (!tr_count)
				break;
			tb_end_mark = sm;
			/* UpdateTableInfo */
			lt.num_col = row_list->max_cell_count_in_line;
			lt.num_row = row_list->row_count;
			lt.tb_end_mark = tb_end_mark;
			lt.tb_start_mark = tb_start_mark;
			lt.row_list = row_list;
			row_list = NULL;
			tr_count = 0;
			break;
		}

		/* Recursive handling of tables in tables */
		if ((sm->type == M_TABLE) && !sm->is_end) {
			/* A table in table or missing </TABLE> */
			TableInfo *tt;

			if (!tr_start_found) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
						"<TABLE> is outside a <TR>\n");
#endif
				/* End Current table */
				mark = GetMarkRec();
				mark->type = M_TABLE;
				mark->is_end = 1;
				/* Link it into markup list */
				mark->next = sm;
				psm->next = mark;
				sm = mark;
				continue;
			}
			if (!td_start_found) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
						"<TABLE> is outside a <TD>\n");
#endif
				/* Let's pretend by faking a <TD> */
				mark = GetMarkRec();
				mark->type = M_TABLE_DATA;
				mark->is_end = 0;
				mark->start = GetMarkText("TD");
				/* Link it into markup list */
				mark->next = sm;
				psm->next = mark;
				sm = mark;
				continue;
			}

			/* Use current cell alignment */
			save_align = pcc->div;
			if (align == HALIGN_CENTER) {
				pcc->div = DIV_ALIGN_CENTER;
			} else if (align == HALIGN_RIGHT) {
				pcc->div = DIV_ALIGN_RIGHT;
			} else if (align == HALIGN_LEFT) {
				pcc->div = DIV_ALIGN_LEFT;
			}
			tt = FirstPassTable(hw, sm, pcc);  /* Be recursive */

			pcc->div = save_align;
			if (!tt) {
#ifndef DISABLE_TRACE
				if (tableTrace || reportBugs)
					fprintf(stderr,
						"Buggy Table in Table!\n");
#endif
				sm->type = M_BUGGY_TABLE; /* Change type */
							  /* and give up */
			} else {
				sm->t_p1 = tt;
				psm = tt->tb_end_mark;
				sm = tt->tb_end_mark->next;
				continue;
			}
		}
		psm = sm;
		sm = sm->next;
	}
	if (!tb_end_mark) {
#ifndef DISABLE_TRACE
		if (tableTrace || reportBugs)
			fprintf(stderr, "Table end not found!\n");
#endif
		if (col_list)
			FreeColList(col_list);
		if (row_list)
			FreeRowlist(row_list);
		return NULL;
	}
	/* Correct invalid column spanning caused by HTML like:
	 *  <TABLE>
	 *	<TR ROWSPAN=2><TD></TD><TD></TD><TD></TD></TR>
	 *	<TR><TD COLSPAN=3><TD></TR>
	 *  </TABLE>
	 */
	for (i = 0; i < lt.num_row; i++) {
	    for (j = 0; j < lt.num_col; j++) {
		if ((lt.row_list->cells_lines[i][j].colspan + j) > lt.num_col) {
		    lt.row_list->cells_lines[i][j].colspan = lt.num_col - j;
#ifndef DISABLE_TRACE
		    if (tableTrace || reportBugs)
			fprintf(stderr,	"Cell colspan reduced.\n");
#endif
		}
	    }
	}
	t = (TableInfo *) calloc(1, sizeof(TableInfo));
	CHECK_OUT_OF_MEM(t);
	*t = lt;

	if (start_col_info) {
		MarkInfo *tmp = end_col_info->next;

		end_col_info->next = NULL;
		apply_colinfo(start_col_info, t);
		end_col_info->next = tmp;
#ifndef DISABLE_TRACE
		if (tableTrace)
			show_colinfo(t);
#endif
	}
	return t;
}

static void EstimateMinMaxTable(HTMLWidget hw, TableInfo *t,
				PhotoComposeContext *orig_pcc)
{
	PhotoComposeContext deb_pcc, fin_pcc;
	MarkInfo *extra, *last_extra;
	CellStruct *line, *cptr;
	CellStruct cell;
	int i, j, k, l, it, min_x, max_x;
	int line_min_w, line_max_w;
	int estimate_height = 0;
	int form_is_cw = 0;
	int h_row = 0;
	WidgetInfo *save_wptr = hw->html.widget_list;
	DescRec *save_DescType;

	/* Get the cached widget stuff out of the way */
	hw->html.widget_list = NULL;

	/* Save form status for hack */
	if (orig_pcc->in_form && orig_pcc->cur_form &&
	    orig_pcc->cur_form->cw_only)
		form_is_cw = 1;
	deb_pcc = *orig_pcc;
	deb_pcc.cw_only = True;
	deb_pcc.x = deb_pcc.y = 0;
	deb_pcc.is_bol = 1;
	deb_pcc.pf_lf_state = 1;
	deb_pcc.nobr = 0;
	deb_pcc.eoffsetx = 0;
	deb_pcc.left_margin = 0;
	deb_pcc.float_left = NULL;
	deb_pcc.right_margin = 0;
	deb_pcc.float_right = NULL;
	deb_pcc.have_space_after = 0;
	deb_pcc.cur_baseline = 0;
	deb_pcc.cur_line_height = 0;
	deb_pcc.max_line_ascent = 0;
	deb_pcc.in_table = 1;

	/* If both table width and height specified, then use the
	 * width so that the height calculation is more accurate.
	 * Must have width, if have height.
	 */
	if (t->absolute_width && t->height) {
		deb_pcc.cur_line_width = t->absolute_width;
	} else {
		t->height = 0;
	}
	if (t->relative_width)
		deb_pcc.cur_line_width =
			   (t->relative_width * orig_pcc->cur_line_width) / 100;

	t->col_max_w = (int *) calloc(t->num_col, sizeof(int));
	CHECK_OUT_OF_MEM(t->col_max_w);
	t->col_min_w = (int *) calloc(t->num_col, sizeof(int));
	CHECK_OUT_OF_MEM(t->col_min_w);
	t->col_w = (int *) calloc(t->num_col, sizeof(int));
	CHECK_OUT_OF_MEM(t->col_w);
	t->col_req_w = (int *) calloc(t->num_col, sizeof(int));
	CHECK_OUT_OF_MEM(t->col_req_w);
	t->col_abs_w = (int *) calloc(t->num_col, sizeof(int));
	CHECK_OUT_OF_MEM(t->col_abs_w);

	/** Done by calloc
	for (i = 0; i < t->num_col; i++) {
		t->col_max_w[i] = 0;
     		t->col_min_w[i] = 0;
		t->col_w[i] = 0;
		t->col_req_w[i] = 0;
		t->col_abs_w[i] = 0;
	}
	**/

	extra = t->first_tr_mark;
     
	/* Execute HTML between <TABLE> and first <TR> or <CAPTION> */
	if (t->start_other_mark) {
		FormatChunk(hw, t->start_other_mark, t->end_other_mark,
			    &deb_pcc);
		deb_pcc.cur_line_height = 0;
		deb_pcc.is_bol = 1;
		deb_pcc.pf_lf_state = 1;
		deb_pcc.x = deb_pcc.y = 0;
	}
	/* Do caption to get approximate height */
        if (t->caption_start_mark) {
		int save = hw->html.percent_vert_space;

		if (t->captionIsLegend ||
		    (t->captionAlignment == VALIGN_BOTTOM)) {
			if (!t->captionIsLegend)
				hw->html.percent_vert_space = 40;
			ConditionalLineFeed(hw, 2, &deb_pcc);
			FormatChunk(hw, t->caption_start_mark,
				    t->caption_end_mark, &deb_pcc);
			ConditionalLineFeed(hw, 1, &deb_pcc);
		} else {
	               	ConditionalLineFeed(hw, 1, &deb_pcc);
                	FormatChunk(hw, t->caption_start_mark,
                		    t->caption_end_mark, &deb_pcc);
                	ConditionalLineFeed(hw, 1, &deb_pcc);
			/* Need less than full blank line */
			hw->html.percent_vert_space = 40;
                	ConditionalLineFeed(hw, 2, &deb_pcc);
		}
		hw->html.percent_vert_space = save;
		estimate_height = deb_pcc.y;
		deb_pcc.cur_line_height = 0;
		deb_pcc.is_bol = 1;
		deb_pcc.pf_lf_state = 1;
		deb_pcc.x = deb_pcc.y = 0;
	}

	/* Caption and other mark stuff may give them values */
	deb_pcc.computed_min_x = deb_pcc.computed_max_x = 0;
	deb_pcc.computed_maxmin_x = 0;

	for (i = 0; i < t->num_row; i++) {
		line = t->row_list->cells_lines[i];
		h_row = 0;
		for (j = 0; j < t->num_col; ) {	 /* For each element... */
			cell = line[j];		 /* One element */
			/* Do crap between table cells */
			if ((cell.cell_type == M_TABLE_DATA) ||
			    (cell.cell_type == M_TABLE_HEADER)) {
			    FormatChunk(hw, extra, cell.td_start, &deb_pcc);
			    deb_pcc.cur_line_height = 0;
			    deb_pcc.is_bol = 1;
			    deb_pcc.pf_lf_state = 1;
			    deb_pcc.x = deb_pcc.y = 0;
			    extra = cell.td_end->next;
			    last_extra = cell.td_end;
			}
			fin_pcc = deb_pcc;
			fin_pcc.at_top = True;
			it = j;
			save_DescType = DescType;
			if (cell.cell_type == M_TABLE_HEADER) {
			    PushFont(hw, &fin_pcc);
			    fin_pcc.cur_font = hw->html.bold_font;
			}

			FormatChunk(hw, cell.td_start, cell.td_end, &fin_pcc);

			/* Get to beginning of line for correct y */
			/* Floating stuff already reset by FormatChunk */
		        ConditionalLineFeed(hw, 1, &fin_pcc);
			PopFontSaved(hw, &fin_pcc);
			/* Reset list stuff */
			if (DescType->next && (DescType != save_DescType)) {
			    DescRec *tmp = DescType;
			    DescRec *dptr;

			    /* Check if too much already removed */
			    while (tmp && (tmp != save_DescType))
				tmp = tmp->next;

			    /* Remove stuff added while in table */
			    while (tmp && DescType->next &&
				   (DescType != save_DescType)) {
				dptr = DescType;
				DescType = DescType->next;
				free(dptr);
			    }
			}
			if (fin_pcc.computed_maxmin_x > fin_pcc.computed_min_x)
			    fin_pcc.computed_min_x = fin_pcc.computed_maxmin_x;
			/* Use maximun or absolute as minimum if nowrapping */
			if (cell.nowrap) {
			    if (cell.absolute_width > fin_pcc.computed_min_x) {
				fin_pcc.computed_min_x = cell.absolute_width;
			    } else if (!cell.absolute_width) {
				fin_pcc.computed_min_x = fin_pcc.computed_max_x;
			    }
			}
			if (cell.absolute_width) {
			    if (cell.absolute_width >= fin_pcc.computed_min_x) {
				/* Always use if very small */
				if (cell.absolute_width < 3)
				    fin_pcc.computed_min_x =cell.absolute_width;
				fin_pcc.computed_max_x = cell.absolute_width;
			    } else {
				fin_pcc.computed_max_x = fin_pcc.computed_min_x;
			    }
			}
			if (cell.colspan > 1) {
			    min_x = max_x = 0;
			} else {
			    min_x = fin_pcc.computed_min_x;
			    max_x = fin_pcc.computed_max_x;
			}
			for (k = 0; k < cell.colspan; k++) {
			    line[j].min_width = min_x;
			    line[j].max_width = max_x;
			    if (t->col_min_w[j] < line[j].min_width)
				t->col_min_w[j] = line[j].min_width;
			    if (t->col_max_w[j] < line[j].max_width)
				t->col_max_w[j] = line[j].max_width;
			    if (cell.absolute_width) {
				t->col_req_w[j] = -1;
				if (cell.absolute_width >= t->col_min_w[j]) {
				    t->col_abs_w[j] = cell.absolute_width;
				} else {
				    t->col_abs_w[j] = t->col_min_w[j];
				}
				if (t->col_max_w[j] > t->col_min_w[j])
				    t->col_max_w[j] = t->col_min_w[j];
				if (t->col_max_w[j] < t->col_abs_w[j])
				    t->col_max_w[j] = t->col_abs_w[j];
			    } else if (cell.relative_width) {
				t->col_req_w[j] = cell.relative_width;
			    }
			    j++;
			}
			if ((cell.cell_type == M_TABLE_DATA) ||
			    (cell.cell_type == M_TABLE_HEADER)) {
			    line[it].min_width = fin_pcc.computed_min_x;
			    line[it].max_width = fin_pcc.computed_max_x;
			}
			if (fin_pcc.y > h_row)
			    h_row = fin_pcc.y;
			deb_pcc.cur_form = fin_pcc.cur_form;
			deb_pcc.in_form = fin_pcc.in_form;
		}
		for (j = 0; j < t->num_col; j++)
			line[j].height = h_row;
		estimate_height += h_row;
	}
	/* Do crap after last table cell */
	if (last_extra != t->tb_end_mark)
		last_extra = last_extra->next;
	FormatChunk(hw,	last_extra, t->tb_end_mark, &deb_pcc);

	/* Handle table height request.  Currently only implemented
	 * for tables with no row spanning in column one.  Also only
	 * takes account of requested cell heights in column one.
	 */
	estimate_height += t->num_row * 2 * (t->borders + t->cellpadding);
	estimate_height += (t->num_row + 1) * t->cellspacing;
	estimate_height += 2 * t->outer_border;
#ifndef DISABLE_TRACE
	if (tableTrace) {
		fprintf(stderr, "Table reguested height = %d\n", t->height);
		fprintf(stderr, "Table estimated height = %d\n",
			estimate_height);
	}
#endif
	if (estimate_height < t->height) {
		/* Is there any row spanning in column one? */
		j = 0;
		for (i = 0; i < t->num_row; i++) {
			cptr = &t->row_list->cells_lines[i][0];
			if ((cptr->cell_type != M_TABLE_DATA) &&
			    (cptr->cell_type != M_TABLE_HEADER))
				break;
			if (cptr->req_height > 0)
				j++;
		}
		/* Bring cell heights up to requested heights, if any */
		if (j) {
			for (k = 0; k < t->num_row; k++) {
				cptr = &t->row_list->cells_lines[k][0];
				if (cptr->req_height > cptr->height) {
					estimate_height += cptr->req_height -
								   cptr->height;
					cptr->height = cptr->req_height;
				}
			}
		}
		if ((i == t->num_row) && (estimate_height < t->height)) {
			int wanted_h = t->height - estimate_height;
			int add_h;

			/* Was there at least one requested cell height,
			 * but not for all cells? */
			if (j && (j != t->num_row)) {
				add_h = wanted_h / (t->num_row - j);
				if (!add_h)
					add_h = 1;
				for (i = 0; i < t->num_row; i++) {
					cptr = &t->row_list->cells_lines[i][0];
					if (!cptr->req_height)
						cptr->treq_height =
							   cptr->height + add_h;
				}
			} else {
				add_h = wanted_h / t->num_row;
				if (!add_h)
					add_h = 1;
				for (i = 0; i < t->num_row; i++) {
					cptr = &t->row_list->cells_lines[i][0];
					cptr->treq_height = cptr->height +add_h;
				}
			}
		}
	}
	/* A hack to handle cells which span otherwise too narrow columns */
	for (i = 0; i < t->num_row; i++) {
		line = t->row_list->cells_lines[i];
		for (j = 0; j < t->num_col; j++) {
			if ((line[j].colspan > 1) && 
			    ((line[j].cell_type == M_TABLE_DATA) ||
			     (line[j].cell_type == M_TABLE_HEADER))) {
				int ncol = 0;
				int tmp_w = 0;
				int tmp_maxw = 0;
				int tmp_a, jk;

				for (k = 0; k < line[j].colspan; k++) {
				    jk = j + k;
				    tmp_w += t->col_min_w[jk];
				    tmp_maxw += t->col_max_w[jk];
				    if (t->col_req_w[jk] != -1)
					ncol++;
				}
				tmp_w += (line[j].colspan - 1) *
					  ((2 * (t->cellpadding + t->borders)) +
					  t->cellspacing);
				tmp_maxw += (line[j].colspan - 1) *
					  ((2 * (t->cellpadding + t->borders)) +
					  t->cellspacing);
				/* First bring minimums up to absolutes */
				if (tmp_w < line[j].min_width) {
				    tmp_a = line[j].min_width - tmp_w;
				    for (k = 0; k < line[j].colspan; k++) {
					jk = j + k;
					l = t->col_abs_w[jk] -
					    t->col_min_w[jk];
					if (l > 0) {
					    if (l < tmp_a) {
						t->col_min_w[jk] =
							       t->col_abs_w[jk];
						tmp_a -= l;
						tmp_w += l;
					    } else {
						t->col_min_w[jk] += tmp_a;
						tmp_w += tmp_a;
						break;
					    }
					}
				    }
				}
				/* Don't add to columns with absolute width */
				if (ncol) {
				    if (tmp_w < line[j].min_width) {
					tmp_w = line[j].min_width - tmp_w;
					tmp_a = tmp_w;
					tmp_w = tmp_w / ncol;
					/* Get remainder, if any */
					tmp_a = tmp_a - (tmp_w * ncol);
					for (k = 0; k < line[j].colspan; k++) {
					    jk = j + k;
					    if (t->col_req_w[jk] != -1) {
						t->col_min_w[jk] += tmp_w;
						/* Spread remainder */
						if (tmp_a) {
						    t->col_min_w[jk] += 1;
						    tmp_a -= 1;
						}
					    }
					}
				    }
				    if (tmp_maxw < line[j].max_width) {
					tmp_maxw = line[j].max_width - tmp_maxw;
					tmp_maxw = tmp_maxw / ncol;
					for (k = 0; k < line[j].colspan; k++) {
					    if (t->col_req_w[j + k] != -1)
						t->col_max_w[j + k] += tmp_maxw;
					}
				    }
				/* unless we absolutely have to */
				} else {
				    if (tmp_w < line[j].min_width) {
					tmp_w = line[j].min_width - tmp_w;
					tmp_a = tmp_w;
					tmp_w = tmp_w / line[j].colspan;
					/* Get remainder, if any */
					tmp_a -= tmp_w * line[j].colspan;
					for (k = 0; k < line[j].colspan; k++) {
					    jk = j + k;
					    t->col_min_w[jk] += tmp_w;
					    /* Spread remainder */
					    if (tmp_a) {
						t->col_min_w[jk] += 1;
						tmp_a -= 1;
					    }
					    if (t->col_req_w[jk] == -1)
						t->col_abs_w[jk] =
							       t->col_min_w[jk];
					}
				    }
				    if (tmp_maxw < line[j].max_width) {
					tmp_maxw = line[j].max_width - tmp_maxw;
					tmp_maxw = tmp_maxw / line[j].colspan;
					for (k = 0; k < line[j].colspan; k++)
					    t->col_max_w[j + k] += tmp_maxw;
				    }
				}
			}
			/* Bring Maximum up to > of Minimum and Absolute */
			if (t->col_min_w[j] > t->col_max_w[j])
				t->col_max_w[j] = t->col_min_w[j];
			if (t->col_abs_w[j] > t->col_max_w[j])
				t->col_max_w[j] = t->col_abs_w[j];
		}
	}

	line_min_w = line_max_w = 0;
	j = 2 * t->cellpadding;
	for (i = 0; i < t->num_col; i++) {
		line_min_w += t->col_min_w[i] + j;
		line_max_w += t->col_max_w[i] + j;
	}

	line_min_w += (t->num_col * 2 * t->borders) + (2 * t->outer_border);
	line_max_w += (t->num_col * 2 * t->borders) + (2 * t->outer_border);
	line_min_w += (t->num_col + 1) * t->cellspacing;
	line_max_w += (t->num_col + 1) * t->cellspacing;

	t->min_width = line_min_w;
	t->max_width = line_max_w;
	t->estimate_height = estimate_height;
	hw->html.widget_list = save_wptr;

	/* Hack to handle extraneous </FORM> tags */
	if (form_is_cw && !deb_pcc.cur_form && !deb_pcc.in_form) {
		orig_pcc->cur_form = NULL;
		orig_pcc->in_form = False;
	}
}

void TablePlace(HTMLWidget hw, MarkInfo **mptr, PhotoComposeContext *pcc)
{
	MarkInfo *sm, *mark, *extra, *last_extra;
	TableInfo *t;
	CellStruct *line, *cptr;
	CellStruct cell;
	ElemInfo *eptr;
	PhotoComposeContext line_pcc, work_pcc, tbl_pcc;
	char *tptr;
	int i, j, k;
	int w_table, h_table, max_line_bot, delta;
	int w_in_cell, to_add_col, wanted_w, w;
	int cell_has_bg, row_has_bg, table_has_bg;
	int save_x, save_y, left, wanted;
	int need_start_LF, cell_offset, add_offset;
	int border_pad, x2border_pad;
	int adjx = 0;
	int save_cur_line_width = pcc->cur_line_width;
	int save_cur_baseline = pcc->cur_baseline;
	int save_cur_line_height = pcc->cur_line_height;
	int ori_y = pcc->y;
	int in_table = pcc->in_table;
	unsigned long save_bg;
	FloatRec *tmp_float;
	DescRec *save_DescType;

	/* Point sm to <TABLE> */
	sm = *mptr;			/* Leave mptr alone til done */

	/* Do a pre-pass to count the number of columns and rows, and
	 * to get various table settings.
	 */
	if (!sm->t_p1) {
		sm->t_p1 = t = FirstPassTable(hw, sm, pcc);
		if (!t) {
#ifndef DISABLE_TRACE
			if (tableTrace || reportBugs) 
				fprintf(stderr, "Invalid table structure!\n");
#endif
			return;
		}
	} else {
		t = sm->t_p1;
	}

	/* Remove extra leading space if there is a left aligned table */
	if (pcc->float_left && pcc->float_left->type == 2) {
		if (pcc->x == (pcc->left_margin + pcc->eoffsetx))
			pcc->x -= pcc->float_left->table_extra;
		pcc->cur_line_width += pcc->float_left->table_extra;
		pcc->left_margin -= pcc->float_left->table_extra;
	}

	/*
	 * Once we have a table, we compute the min and max size of each cell,
	 * save the context for each cell, parse between marker, the return
	 * context gives the size.  When doing this NEVER create element.
	 */
	if (!t->min_width)   /* We haven't established the dimensions */
		EstimateMinMaxTable(hw, t, pcc);

	if (t->relative_width) {
		wanted_w = (t->relative_width * pcc->cur_line_width) / 100;
	} else {
		wanted_w = t->absolute_width;
	}

	/* TablePlace has been called inside another table's size computation */
	if (pcc->cw_only) {
		if (!t->relative_width && wanted_w) {
			/* Set table min and max width to the greater of
			 * requested width or table min width */
			if (wanted_w > t->min_width) {
				t->min_width = t->max_width = wanted_w;
			} else {
				t->max_width = t->min_width;
			}
		}
		if (pcc->computed_min_x < (pcc->left_margin + pcc->eoffsetx +
		    t->min_width))
			pcc->computed_min_x = pcc->left_margin + pcc->eoffsetx +
								   t->min_width;
		if (pcc->computed_max_x < pcc->x + t->max_width)
			pcc->computed_max_x = pcc->x + t->max_width;

		/* Advance mark pointer to table end */
		*mptr = t->tb_end_mark;
		return;
	}

	/* If no requested width and room left, then use absolute column
	 * widths if any */
	if (!wanted_w && (t->min_width < pcc->cur_line_width)) {
		left = pcc->cur_line_width - t->min_width;
		for (i = 0; i < t->num_col; i++) {
			wanted = t->col_abs_w[i] - t->col_min_w[i];
			if (wanted > 0) {
				if (left <= wanted) {
					t->col_min_w[i] += left;
					t->min_width += left;
					break;
				} else {
					t->col_min_w[i] += wanted;
					t->min_width += wanted;
					left -= wanted;
				}
			}
		}
	}
  /*******
   3 cases:

	1. t->min_width >= viewable area or is a requested size ====>
	   allocate the minimum for each column

	2. t->max_width < viewable area  =====>
	   allocate the maximum size for each column

	3. t->min_width < viewable area < t->max_width ====>
	   allocate the minimum for each column and let later
	   calculations expand to viewable area

	Note that t->col_max_w[i] = t->col_min_w[i] when absolute column
	width has been specified.

	Now, calculate t->col_w[i] according to those 3 cases.
   *******/

	/* case 1 */
	if ((t->min_width >= pcc->cur_line_width) || wanted_w) {
		for (i = 0; i < t->num_col; i++)
			t->col_w[i] = t->col_min_w[i];
	/* case 2 */
	} else if (t->max_width < pcc->cur_line_width) {
		for (i = 0; i < t->num_col; i++)
			t->col_w[i] = t->col_max_w[i];
	/* case 3 */
	} else {
		/* Let later calculations do it */
		wanted_w = pcc->cur_line_width;
		for (i = 0; i < t->num_col; i++)
			t->col_w[i] = t->col_min_w[i];
	}

	/* Now we can calculate the width of the table */
	w_table = 0;
	for (i = 0; i < t->num_col; i++)
		w_table += t->col_w[i];

	w_table += t->num_col * (2 * t->cellpadding);
	w_table += (t->num_col + 1) * t->cellspacing;
	w_table += (t->num_col * 2 * t->borders) + (2 * t->outer_border);

#ifndef DISABLE_TRACE
	if (tableTrace) {
		fprintf(stderr, "\ncur_line_width = %d\n", pcc->cur_line_width);
		fprintf(stderr, "min width = %d, max width = %d\n",
			t->min_width, t->max_width);
		fprintf(stderr, "requested width = %d, computed width = %d\n",
			wanted_w, w_table);
		fprintf(stderr, "Computed column widths:\n");
		for (i = 0; i < t->num_col; i++)
			fprintf(stderr, "|%d", t->col_w[i]);
		fprintf(stderr, "|\n");
		fprintf(stderr, "Maximum column widths:\n");
		for (i = 0; i < t->num_col; i++)
			fprintf(stderr, "|%d", t->col_max_w[i]);
		fprintf(stderr, "|\n");
		fprintf(stderr, "Minimum column widths:\n");
		for (i = 0; i < t->num_col; i++)
			fprintf(stderr, "|%d", t->col_min_w[i]);
		fprintf(stderr, "|\n");
	}
#endif
	/* If needed, add width to each column */
	if (wanted_w > w_table) {
		int max_left = 0;
		float max_percent;
		int ncol;

		left = wanted_w - w_table;
		/* First, try to give each column its requested width. */
		/* Requested it in pixels */
		for (i = 0; i < t->num_col; i++) {
			if (t->col_abs_w[i]) {
				wanted = t->col_abs_w[i] - t->col_w[i];
				if (wanted > 0) {
					if (left <= wanted) {
						t->col_w[i] += left;
						w_table += left;
						left = 0;
						break;
					} else {
						t->col_w[i] += wanted;
						w_table += wanted;
						left -= wanted;
					}
				}
			}
		}
		/* Requested a percentage */
		if (left) { 
			for (i = 0; i < t->num_col; i++) {
				if (t->col_req_w[i] > 0) {
					wanted = ((wanted_w * t->col_req_w[i]) /
						  100) - t->col_w[i];
					if (wanted > 0) {
						if (left <= wanted) {
							t->col_w[i] += left;
							w_table += left;
							left = 0;
							break;
						} else {
							t->col_w[i] += wanted;
							w_table += wanted;
							left -= wanted;
						}
					}
				}
		 	}
		}
		/* If room left, maximize each column with no requested width
		 * as needed */
		if (left) {
			for (i = 0; i < t->num_col; i++) {
				if (!t->col_req_w[i] &&
				    (t->col_w[i] < t->col_max_w[i]))
					max_left += t->col_max_w[i] -
								    t->col_w[i];
			}
			if (max_left && (max_left <= left)) {
				w_table += max_left;
				for (i = 0; i < t->num_col; i++) {
					if (!t->col_req_w[i] &&
					    (t->col_w[i] < t->col_max_w[i]))
						t->col_w[i] = t->col_max_w[i];
				}
			} else if (max_left) {
				max_percent = left / (float)max_left;
				for (i = 0; i < t->num_col; i++) {
					if (!t->col_req_w[i] &&
					    (t->col_w[i] < t->col_max_w[i])) {
						j = (t->col_max_w[i] -
						     t->col_w[i]) * max_percent;
						w_table += j;
						t->col_w[i] += j;
					}
				}
			}
		}
		/* Still need more?  Expand columns with no requested width */
		if (wanted_w > w_table) {
			ncol = 0;
			for (i = 0; i < t->num_col; i++) {
				if (!t->col_req_w[i])
					ncol++;
			}
			to_add_col = (wanted_w - w_table) / (ncol ? ncol : 1);
			for (i = 0; i < t->num_col; i++) {
				if (!t->col_req_w[i]) {
					t->col_w[i] += to_add_col;
					w_table += to_add_col;
				}
			}
		}
		/* If still more, then add to columns with absolute width */
		if (wanted_w > w_table) {
			ncol = 0;
			for (i = 0; i < t->num_col; i++) {
				if (t->col_abs_w[i])
					ncol++;
			}
			to_add_col = (wanted_w - w_table) / (ncol ? ncol : 1);
			for (i = 0; i < t->num_col; i++) {
				if (t->col_abs_w[i]) {
					t->col_w[i] += to_add_col;
					w_table += to_add_col;
				}
			}
		}
		/* If still not enough, then add to all columns regardless */
		if (wanted_w > w_table) {
			to_add_col = (wanted_w - w_table) / t->num_col;
			for (i = 0; i < t->num_col; i++) {
				t->col_w[i] += to_add_col;
				w_table += to_add_col;
			}
		}
	}
	/* Now retry percentage allocation multiple times, if room remaining */
	if (wanted_w) {
		/* Should always be equal to w_table by now */
		w = wanted_w;
	} else {
		/* No requested width, so use viewable area */
		w = pcc->cur_line_width;
	}
	j = 0;
	while ((w_table < w) && (j < 40)) {
		int totcolwidth = 0;

		left = w - w_table;
		/* Get total of columns minus all the border, etc. stuff */
		for (i = 0; i < t->num_col; i++)
			totcolwidth += t->col_w[i];

		/* Try to give each column its requested percentage */
		k = 0;
		for (i = 0; i < t->num_col; i++) {
			if (t->col_req_w[i] > 0) {
				wanted = ((totcolwidth * t->col_req_w[i])/100) -
					 			    t->col_w[i];
				if (wanted > 0) {
					k = 1;
					if (left <= wanted) {
						t->col_w[i] += left;
						w_table += left;
						left = 0;
						break;
					} else {
						t->col_w[i] += wanted;
						w_table += wanted;
						left -= wanted;
					}
				}
			}
		}
		if (!k || !left)  /* Nothing left to adjust or no percentages */
			break;
		j++;
#ifndef DISABLE_TRACE
		if (tableTrace) {
			fprintf(stderr, "Pass %d percent adjusted width = %d\n",
				j, w_table);
			fprintf(stderr, "Adjusted column widths:\n");
			for (i = 0; i < t->num_col; i++)
				fprintf(stderr, "|%d", t->col_w[i]);
			fprintf(stderr, "|\n");
		}
#endif
	}
#ifndef DISABLE_TRACE
	if (tableTrace) {
		fprintf(stderr, "Final width = %d\n", w_table);
		fprintf(stderr, "Final column widths:\n");
		for (i = 0; i < t->num_col; i++)
			fprintf(stderr, "|%d", t->col_w[i]);
		fprintf(stderr, "|\n");
	}
#endif

	/* Get to left margin if not there */
	if (!pcc->is_bol)
		ConditionalLineFeed(hw, 1, pcc);

	if (pcc->max_width_return < (w_table + pcc->x + pcc->right_margin))
		pcc->max_width_return = w_table + pcc->x + pcc->right_margin;

	/* Mark table start with special CR */
	eptr = CreateElement(hw, E_CR, pcc->cur_font, pcc->x, pcc->y,
			     0, pcc->cur_line_height, pcc->cur_baseline, pcc); 
	eptr->table_data = t;

	/* Execute HTML between <TABLE> and <CAPTION> */
	if (t->start_other_mark && t->other_before_caption) {
		pcc->ignore_float = 1;
		FormatChunk(hw, t->start_other_mark, t->end_other_mark,	pcc);
		pcc->ignore_float = 0;
	}

	/* Put top caption here */
	if (t->caption_start_mark &&
	    ((t->captionAlignment != VALIGN_BOTTOM) || t->captionIsLegend)) {
		DivAlignType tmp;

		pcc->cur_line_width = w_table;
		pcc->ignore_float = 1;
		if (t->captionIsLegend) {
			ConditionalLineFeed(hw, 2, pcc);
		} else {
			ConditionalLineFeed(hw, 1, pcc);
		}
		tmp = pcc->div;
		pcc->div = DIV_ALIGN_CENTER;
		if (t->captionAlignment == HALIGN_LEFT) {
			pcc->div = DIV_ALIGN_LEFT;
		} else if (t->captionAlignment == HALIGN_RIGHT) {
			pcc->div = DIV_ALIGN_RIGHT;
		}
		FormatChunk(hw, t->caption_start_mark, t->caption_end_mark,pcc);
		ConditionalLineFeed(hw, 1, pcc);
		if (!t->captionIsLegend) {
			int save = hw->html.percent_vert_space;

			/* Need less than full blank line */
			hw->html.percent_vert_space = 40;
			ConditionalLineFeed(hw, 2, pcc);
			hw->html.percent_vert_space = save;
		}
		pcc->ignore_float = 0;
		pcc->div = tmp;
		pcc->cur_line_width = save_cur_line_width;
	}
	/* Execute HTML between <TABLE> or <CAPTION> and first <TR> */
	need_start_LF = 0;
	if (t->start_other_mark && !t->other_before_caption) {
		pcc->in_table = -1;
		pcc->ignore_float = 1;
		FormatChunk(hw, t->start_other_mark, t->end_other_mark, pcc);
		pcc->ignore_float = 0;
		if (pcc->in_table == -2) {
			/* There was a <FORM>.  Check if form stuff in row 1 */
			line = t->row_list->cells_lines[0];
			for (i = 0; (i < t->num_col) && !need_start_LF; i++) {
				cell = line[i];
				mark = cell.td_start;
				while (mark && (mark != cell.td_end)) {
					if ((mark->type == M_INPUT) ||
					    (mark->type == M_TEXTAREA) ||
					    (mark->type == M_SELECT)) {
						/* Start row 1 with linefeed */
						need_start_LF = 1;
						break;
					}
					mark = mark->next;
				}
			}
		}
	}
	pcc->in_table = 1;		/* In table */
	tbl_pcc = *pcc;
	
	/* Do table background color */
	table_has_bg = 0;
	if (tptr = ParseMarkTag(sm->start, MT_TABLE, "BGCOLOR")) {
		hw_do_color(hw, "tcolor", tptr, &tbl_pcc);
		table_has_bg = 1;
		free(tptr);
	}

	/* Now create the table */

	tbl_pcc.x += t->outer_border;
	tbl_pcc.y += t->outer_border + t->cellspacing;
	tbl_pcc.eoffsetx = tbl_pcc.x;
	tbl_pcc.cur_line_height = 0;
	tbl_pcc.cur_baseline = 0;
	tbl_pcc.max_line_ascent = 0;
	tbl_pcc.is_bol = 1;
	tbl_pcc.pf_lf_state = 1;
	tbl_pcc.nobr = 0;
	tbl_pcc.at_top = True;
	tbl_pcc.float_left = NULL;
	tbl_pcc.float_right = NULL;
	max_line_bot = tbl_pcc.y;
	extra = t->first_tr_mark;
	border_pad = t->cellpadding + t->borders;
	x2border_pad = border_pad * 2;

	for (i = 0; i < t->num_row; i++) {
	    line_pcc = tbl_pcc;
	    line = t->row_list->cells_lines[i];
	    cell_offset = t->cellspacing;
	    /* Do row background color */
	    row_has_bg = 0;
	    for (j = 0; j < t->num_col; j++) {  /* Skip over M_TD_CELL_FREE */
		if ((line[j].cell_type == M_TABLE_DATA) ||
		    (line[j].cell_type == M_TABLE_HEADER))
		    break;
	    }
	    if (hw->html.body_colors && (j < t->num_col)) {
		tptr = ParseMarkTag(line[j].tr_start->start,
				    MT_TABLE_ROW, "bgcolor");
		if (tptr) {
		    hw_do_color(hw, "trcolor", tptr, &line_pcc);
		    free(tptr);
		    row_has_bg = 1;
		}
	    }
	    for (j = 0; j < t->num_col; j++) {  /* For each cell */
		w_in_cell = t->col_w[j]; 
		cell = line[j];		/* Get a cell */
		/* Do crap between table cells */
		if ((cell.cell_type == M_TABLE_DATA) ||
		    (cell.cell_type == M_TABLE_HEADER)) {
		    work_pcc = line_pcc;
		    FormatChunk(hw, extra, cell.td_start, &work_pcc);
		    line_pcc.cur_form = work_pcc.cur_form;
		    line_pcc.in_form = work_pcc.in_form;
		    line_pcc.widget_id = work_pcc.widget_id;
		    line_pcc.element_id = work_pcc.element_id;
		    line_pcc.last_progressive_ele =
						  work_pcc.last_progressive_ele;
		    extra = cell.td_end->next;
		    last_extra = cell.td_end;
		}
		work_pcc = line_pcc;	/* Grab one to work with */
		add_offset = w_in_cell + x2border_pad;
		cell.width = w_in_cell + x2border_pad;
		cell.y = line_pcc.y; 
		cell.height = x2border_pad;
		cell.line_bottom = line_pcc.y + border_pad;

		switch (cell.cell_type) {
		    case M_TD_CELL_PAD:
			cell.x = cell_offset + line_pcc.eoffsetx;
			/* Propagate height and line_bottom from
			 * starting cell span */
			cell.line_bottom =
				      t->row_list->cells_lines[i - cell.back_rs]
						 [j - cell.back_cs].line_bottom;
			cell.height = t->row_list->cells_lines[i - cell.back_rs]
						      [j - cell.back_cs].height;
			break;
		    case M_TD_CELL_FREE:
		    case M_TD_CELL_PROPAGATE:
			cell.x = cell_offset + line_pcc.eoffsetx;
			break;
		    case M_TABLE_DATA:
		    case M_TABLE_HEADER:
			for (k = 1; k < cell.colspan; k++)
			    w_in_cell += t->col_w[j + k];

			/* Adjust if spans columns */
			w_in_cell += (cell.colspan - 1) *
			    	     (x2border_pad + t->cellspacing);
			work_pcc.left_margin = border_pad;
			work_pcc.right_margin = t->cellpadding;
			work_pcc.cur_line_width = w_in_cell;
			work_pcc.eoffsetx = line_pcc.eoffsetx + cell_offset;
			work_pcc.x = work_pcc.eoffsetx + work_pcc.left_margin;
			work_pcc.y = line_pcc.y + border_pad;
			work_pcc.have_space_after = 0;
			if (cell.cell_type == M_TABLE_HEADER) {
			    PushFont(hw, &work_pcc);
			    work_pcc.cur_font = hw->html.bold_font;
			    work_pcc.div = DIV_ALIGN_CENTER;
			} else {
			    work_pcc.div = DIV_ALIGN_NONE;
			}
			if (cell.halignment == HALIGN_CENTER) {
			    work_pcc.div = DIV_ALIGN_CENTER;
			} else if (cell.halignment == HALIGN_RIGHT) {
			    work_pcc.div = DIV_ALIGN_RIGHT;
			} else if (cell.halignment == HALIGN_LEFT) {
			    work_pcc.div = DIV_ALIGN_LEFT;
			}
			work_pcc.valign = cell.valignment;
			/* Still needed for cell alignment, etc. */
			eptr = CreateElement(hw, E_CR, work_pcc.cur_font,
                			     work_pcc.x, work_pcc.y,        
                			     0, work_pcc.cur_line_height,
                			     work_pcc.cur_baseline, &work_pcc);
			save_x = work_pcc.x;
			save_y = work_pcc.y;
			cell.start_elem = hw->html.last_formatted_elem;

			cell_has_bg = 0;
			if (hw->html.body_colors) {
			    if (cell.cell_type == M_TABLE_DATA) {
				tptr = ParseMarkTag(cell.td_start->start,
					            MT_TABLE_DATA, "bgcolor");
			    } else {
				tptr = ParseMarkTag(cell.td_start->start,
						    MT_TABLE_HEADER, "bgcolor");
			    }
			    if (tptr) {
				cell_has_bg = 1;
				hw_do_color(hw, "tdcolor", tptr, &work_pcc);
				free(tptr);
			    }
			    if (cell_has_bg || row_has_bg || table_has_bg) {
				eptr = CreateElement(hw, E_CELL_TABLE,
						     work_pcc.cur_font,
						     work_pcc.x, work_pcc.y,
						     0, 0, 0, &work_pcc);
				eptr->table_data = t;
				eptr->cell_data =
						&t->row_list->cells_lines[i][j];
				if (!cell_has_bg)
				    /* Inherited the bg */
				    cell_has_bg = 2;
				save_bg = hw->core.background_pixel;
				hw->core.background_pixel = eptr->bg;
			    }
			}
			save_DescType = DescType;

			/* If starting LF needed */
			if (need_start_LF)
			    ConditionalLineFeed(hw, 2, &work_pcc);

			FormatChunk(hw, cell.td_start, cell.td_end, &work_pcc);

			PopFontSaved(hw, &work_pcc);

			/* Reset list stuff */
			if (DescType->next && (DescType != save_DescType)) {
			    DescRec *tmp = DescType;
			    DescRec *dptr;

			    /* Check if too much already removed */
			    while (tmp && (tmp != save_DescType))
				tmp = tmp->next;

			    /* Remove stuff added while in table */
			    while (tmp && DescType->next &&
				   (DescType != save_DescType)) {
				dptr = DescType;
				DescType = DescType->next;
				free(dptr);
			    }
			}
			if (cell_has_bg)
			    hw->core.background_pixel = save_bg;
			if ((cell_has_bg == 1) || (save_x != work_pcc.x) ||
			    (save_y != work_pcc.y)) {
			    cell.has_content = True;
			} else {
			    cell.has_content = False;
			}
			ConditionalLineFeed(hw, 1, &work_pcc);
			cell.end_elem = hw->html.last_formatted_elem;
			cell.x = cell_offset + line_pcc.eoffsetx;
			cell.width = w_in_cell + x2border_pad;
			cell.y = line_pcc.y;
			/* Make sure we have biggest y */
			while (work_pcc.float_left) {
			    if (work_pcc.y < work_pcc.float_left->y)
				work_pcc.y = work_pcc.float_left->y;
			    tmp_float = work_pcc.float_left;
			    work_pcc.float_left = work_pcc.float_left->next;
			    free(tmp_float);
			}
			while (work_pcc.float_right) {
			    if (work_pcc.y < work_pcc.float_right->y)
				work_pcc.y = work_pcc.float_right->y;
			    tmp_float = work_pcc.float_right;
			    work_pcc.float_right = work_pcc.float_right->next;
			    free(tmp_float);
			}
			/* Save height of contents */
			cell.content_height = work_pcc.y - save_y;
			/* Do requested height, if any */
			if (cell.content_height < cell.req_height)
			    work_pcc.y += cell.req_height - cell.content_height;

			if ((cell.content_height < cell.treq_height) &&
			    ((h_table + t->cellspacing + border_pad +
			      t->outer_border + cell.content_height) <
			     t->height)) {
			    int hdiff = t->height - (h_table + t->cellspacing +
					          border_pad + t->outer_border +
					          cell.content_height);

			    if (hdiff >= (cell.treq_height -
				          cell.content_height)) {
				work_pcc.y += cell.treq_height -
				    	      cell.content_height;
			    } else {
				work_pcc.y += hdiff;
			    }
			}
			/* Difference of the pcc's to get the height */
			cell.height = work_pcc.y - line_pcc.y +	border_pad;
			cell.line_bottom = work_pcc.y + border_pad;
			if (eptr->type == E_CELL_TABLE) {
			    eptr->height = cell.height;
			    eptr->width = cell.width;
			    eptr->x = cell.x;
			    eptr->y = cell.y;
			}
			line_pcc.widget_id = work_pcc.widget_id;
			line_pcc.element_id = work_pcc.element_id;
			line_pcc.applet_id = work_pcc.applet_id;
			line_pcc.last_progressive_ele =
			    			  work_pcc.last_progressive_ele;
			line_pcc.cur_form = work_pcc.cur_form;
			line_pcc.in_form = work_pcc.in_form;
			line_pcc.max_width_return = work_pcc.max_width_return;
			break;
#ifndef DISABLE_TRACE
		    default:
			if (reportBugs) 
			    fprintf(stderr, "BUG: Bad cell type in TABLE\n");
#endif
		}
		cell_offset += add_offset + t->cellspacing;
		/* If cell_alone or cell_end_of_rowspan */
		if ((cell.rowspan == 1) && (cell.line_bottom > max_line_bot))
		    max_line_bot = cell.line_bottom;
		line[j] = cell;
	    }
	    /* Only needed for first row */
	    need_start_LF = 0;
	    /*
	     * Adjust the height of lone cells, so the bottom rows
	     * get aligned.
	     */
	    for (j = 0; j < t->num_col; j++) {
		if ((line[j].colspan == 1) && (line[j].rowspan == 1)) {
		    /* Adjust the cell height according to max_line_bot */
		    cptr = &t->row_list->cells_lines
			    	     [i - line[j].back_rs][j - line[j].back_cs];
		    delta = max_line_bot - cptr->line_bottom;
		    if (delta > 0 ) {
			cptr->height += delta;
			cptr->line_bottom = max_line_bot;
		    }
		}
	    }
	    /* Stack the height of the table */
	    h_table = max_line_bot - pcc->y;
	    tbl_pcc.y = max_line_bot + t->cellspacing;
	    tbl_pcc.widget_id = line_pcc.widget_id;
	    tbl_pcc.element_id = line_pcc.element_id;
	    tbl_pcc.applet_id = line_pcc.applet_id;
	    tbl_pcc.last_progressive_ele = line_pcc.last_progressive_ele;
	    tbl_pcc.cur_form = line_pcc.cur_form;
	    tbl_pcc.in_form = line_pcc.in_form;
	    tbl_pcc.max_width_return = line_pcc.max_width_return;
	}
	/* Now go back and do vertical alignment in each cell */
	for (i = 0; i < t->num_row; i++) {
		line = t->row_list->cells_lines[i];
		for (j = 0; j < t->num_col; j++) {
			cptr = &line[j];
			if (((cptr->cell_type != M_TABLE_DATA) &&
			     (cptr->cell_type != M_TABLE_HEADER)) ||
			    (cptr->valignment == VALIGN_TOP))
				continue;

			delta = cptr->height - x2border_pad;
			if (cptr->content_height >= delta)
				continue;

			delta -= cptr->content_height;
			if (cptr->valignment == VALIGN_MIDDLE)
				delta = delta / 2;
			if ((delta <= 0) ||
			    (cptr->start_elem == cptr->end_elem))
				continue;

			/* Skip the CR */
			eptr = cptr->start_elem->next;
			if (eptr == cptr->end_elem)
				continue;

			/* Skip the cell's E_CELL_TABLE element if it has one */
			if (eptr && (eptr->type == E_CELL_TABLE))
				eptr = eptr->next;
			while (eptr && (eptr != cptr->end_elem)) {
				eptr->y += delta;
				if (eptr->type == E_WIDGET) {
					int orig_x = pcc->x;
					int orig_y = pcc->y;
					unsigned long orig_bg = pcc->bg;

					/* Already there so don't need markup */
					pcc->x = eptr->x;
					pcc->y = eptr->y;
					pcc->bg = eptr->bg;
					MakeWidget(hw, NULL, pcc,
						   eptr->widget_data->id);
					pcc->bg = orig_bg;
					pcc->x = orig_x;
					pcc->y = orig_y;
				} else if (eptr->type == E_IFRAME) {
					eptr->frame->frame_y = eptr->y;

				/* Adjust adjustments in any tables in cell */
				} else if (eptr->type == E_TABLE) {
					eptr->table_data->valign_adjx += delta;
				}
				eptr = eptr->next;
			}
		}
	}
	/* Do stuff after last table cell */
	if (last_extra != t->tb_end_mark)
		last_extra = last_extra->next;
	tbl_pcc.cur_line_width = pcc->cur_line_width;
	FormatChunk(hw,	last_extra, t->tb_end_mark, &tbl_pcc);

	pcc->widget_id = tbl_pcc.widget_id;
	pcc->element_id = tbl_pcc.element_id;
	pcc->applet_id = tbl_pcc.applet_id;
	pcc->last_progressive_ele = tbl_pcc.last_progressive_ele;
	pcc->cur_form = tbl_pcc.cur_form;
	pcc->in_form = tbl_pcc.in_form;
	pcc->max_width_return = tbl_pcc.max_width_return;

	t->width = w_table;
	h_table += t->cellspacing + t->outer_border;
	t->height = h_table;

	/* Mark the graphical element that wraps around the table */
	eptr = CreateElement(hw, E_TABLE, pcc->cur_font,
			     pcc->x, pcc->y, w_table, h_table, h_table, pcc); 

	eptr->underline_number = 0;  /* Tables can't be underlined */
	eptr->table_data = t;

	/* Advance mark pointer to end of table */
	*mptr = t->tb_end_mark;

	/* Now align it */
	if ((t->align == DIV_ALIGN_CENTER) || (t->align == DIV_ALIGN_RIGHT) ||
	    (t->align == HALIGN_RIGHT)) {
		adjx = pcc->cur_line_width - eptr->width;
		if (t->align == DIV_ALIGN_CENTER)
			adjx = adjx / 2;
		if (adjx > 0) {
			eptr->table_data->align_adjx = adjx;
		} else {
			eptr->table_data->align_adjx = 0;
		}
#ifndef DISABLE_TRACE
		if (tableTrace)
			fprintf(stderr,
				"adjx: %d width: %d cur_line_width: %d\n",
				adjx, eptr->width, pcc->cur_line_width);
#endif
	}
	pcc->div = DIV_ALIGN_NONE;

	/* Get to left margin */
	pcc->pf_lf_state = 1;
	pcc->x = pcc->left_margin + pcc->eoffsetx;
	pcc->y += eptr->height;
	pcc->is_bol = 1;
	pcc->cur_line_height = 0;
	pcc->cur_baseline = 0;
	pcc->at_top = False;

	/* Keep AdjustBaseLine from screwing with table height */
	eptr = CreateElement(hw, E_CR, pcc->cur_font, pcc->x, pcc->y,
			     0, pcc->cur_line_height, pcc->cur_baseline, pcc); 

	/* Put bottom caption here */
	if (t->caption_start_mark && !t->captionIsLegend &&
	    (t->captionAlignment == VALIGN_BOTTOM)) {
		DivAlignType tmp;
		int save = hw->html.percent_vert_space;

		/* Keep LinefeedPlace from messing with floating margins */
		pcc->ignore_float = 1;
		pcc->cur_line_width = w_table;
		/* Force small amount of white space */
		hw->html.percent_vert_space = 40;
		ConditionalLineFeed(hw, 2, pcc);
		hw->html.percent_vert_space = save;
		tmp = pcc->div;
		pcc->div = DIV_ALIGN_CENTER;
		FormatChunk(hw, t->caption_start_mark, t->caption_end_mark,pcc);
		ConditionalLineFeed(hw, 1, pcc);
		pcc->ignore_float = 0;
		pcc->div = tmp;
		pcc->cur_line_width = save_cur_line_width;
		/* Allow following alignment code to process the caption */ 
		eptr = CreateElement(hw, E_CR, pcc->cur_font, pcc->x, pcc->y, 0,
				  pcc->cur_line_height, pcc->cur_baseline, pcc);
	}

	/* Back to the list until special CR and adjust each x with adjx. */
	/* Also fixup any incorrect cell heights due to spanned empty space. */
	while (eptr && ((eptr->type != E_CR) || !eptr->table_data)) {
		if (adjx > 0) {
			eptr->x += adjx;
			if (eptr->type == E_WIDGET) {
				int orig_x = pcc->x;
				int orig_y = pcc->y;
				unsigned long orig_bg = pcc->bg;

				/* It is already there so don't need markup */
				pcc->x = eptr->x;
				pcc->y = eptr->y;
				pcc->bg = eptr->bg;
				MakeWidget(hw, NULL, pcc,
					   eptr->widget_data->id);
				pcc->bg = orig_bg;
				pcc->x = orig_x;
				pcc->y = orig_y;
			} else if (eptr->type == E_IFRAME) {
				eptr->frame->frame_x = eptr->x;
			}
			/* Adjust adjustments in any tables in this table */
			if ((eptr->type == E_TABLE) && (eptr->table_data != t))
				eptr->table_data->align_adjx += adjx;
		}
		if (eptr->type == E_CELL_TABLE)
			eptr->height = eptr->cell_data->height;
		eptr = eptr->prev;
	}

	/* Clear special CR so nested table alignment works */
	if (eptr)
		eptr->table_data = NULL;

	pcc->in_table = in_table;	/* Indicate no longer in this table */

	/* Restore extra leading space if there is a left aligned table */
	if (pcc->float_left && pcc->float_left->type == 2) {
		pcc->x += pcc->float_left->table_extra;
		pcc->cur_line_width -= pcc->float_left->table_extra;
		pcc->left_margin += pcc->float_left->table_extra;
	}

	/* Check if done floating around stuff */
	if ((t->align != HALIGN_LEFT) && (t->align != HALIGN_RIGHT)) {
		while (pcc->float_left && (pcc->y >= pcc->float_left->y)) {
			pcc->left_margin -= pcc->float_left->marg;
			pcc->cur_line_width += pcc->float_left->marg;
			if (pcc->y == pcc->float_left->y)
				pcc->y++;
			tmp_float = pcc->float_left;
			pcc->float_left = pcc->float_left->next;
			free(tmp_float);
			pcc->x = pcc->left_margin + pcc->eoffsetx;
		}
		while (pcc->float_right && (pcc->y >= pcc->float_right->y)) {
			pcc->right_margin -= pcc->float_right->marg;
			pcc->cur_line_width += pcc->float_right->marg;
			if (pcc->y == pcc->float_right->y)
				pcc->y++;
			tmp_float = pcc->float_right;
			pcc->float_right = pcc->float_right->next;
			free(tmp_float);
			pcc->x = pcc->left_margin + pcc->eoffsetx;
		}
	}
	/* Do left/right alignment stuff */
	if ((t->align == HALIGN_LEFT) && ((pcc->x + w_table) <
	     (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width))) {
		tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
		tmp_float->next = pcc->float_left;
		pcc->float_left = tmp_float;
		/* 1 is image, 2 is table */
		tmp_float->type = 2;
		tmp_float->table_extra = XTextWidth(pcc->cur_font, " ", 1);
		w_table += tmp_float->table_extra;  /* Space after it */
		pcc->left_margin += w_table;
		pcc->cur_line_width -= w_table;
		tmp_float->marg = w_table;
		pcc->cur_baseline = save_cur_baseline;
		pcc->cur_line_height = save_cur_line_height;
		pcc->is_bol = 1;
		pcc->pf_lf_state = 1;
		pcc->x += w_table;
		tmp_float->y = pcc->y;
		pcc->y = ori_y;
		/* Keep Adjustbaseline, etc. from messing up */
		eptr = CreateElement(hw, E_CR, pcc->cur_font,
				     pcc->x, pcc->y, 0, pcc->cur_line_height,
				     pcc->cur_baseline, pcc);
#ifndef DISABLE_TRACE
	     	if (tableTrace || htmlwTrace)
			fprintf(stderr, "Float left started in TablePlace\n");
#endif
	} else if ((t->align == HALIGN_RIGHT) && ((pcc->x + w_table) <
	            (pcc->eoffsetx + pcc->left_margin + pcc->cur_line_width))) {
		tmp_float = (FloatRec *)malloc(sizeof(FloatRec));
		tmp_float->next = pcc->float_right;
		pcc->float_right = tmp_float;
		tmp_float->type = 2;
		/* Space before it */
		w_table += XTextWidth(pcc->cur_font, " ", 1);
		pcc->right_margin += w_table;
		pcc->cur_line_width -= w_table;
		tmp_float->marg = w_table;
		pcc->cur_baseline = save_cur_baseline;
		pcc->cur_line_height = save_cur_line_height;
		tmp_float->y = pcc->y;
		pcc->y = ori_y;
		/* Keep Adjustbaseline, etc. from messing up */
		eptr = CreateElement(hw, E_CR, pcc->cur_font,
				     pcc->x, pcc->y, 0, pcc->cur_line_height,
				     pcc->cur_baseline, pcc);
#ifndef DISABLE_TRACE
	     	if (tableTrace || htmlwTrace)
			fprintf(stderr, "Float right started in TablePlace\n");
#endif
	}
#ifndef DISABLE_TRACE
     	if (tableTrace || htmlwTrace)
		TableDump(t);
#endif
}

static GC ttopGC = NULL;
static GC tbotGC;
#define shadowpm_width 2
#define shadowpm_height 2
static char shadowpm_bits[] = {0x02, 0x01};
#define MAX_SEG 128

/* Display table borders */
void TableRefresh(HTMLWidget hw, ElemInfo *eptr)
{
	TableInfo *t = eptr->table_data;
	CellStruct cell;
	CellStruct **cells_lines;
	XPoint pt[6];
	Display *dsp = hw->html.dsp;
	Screen *scn = XtScreen(hw);
	GC ltopGC, lbotGC;
	XSegment segT[MAX_SEG], segB[MAX_SEG];
	int x, y, i, j, iseg, group, do_group;
	int cw1, ch1, cx, cy, ax, ay;
	FrameType ft;
	RulesType rules;

	/* Trace the border of the table, if any */
	if (!t->borders)
		return;
	x = eptr->x - hw->html.scroll_x;
	y = eptr->y - hw->html.scroll_y;
	ax = t->align_adjx - hw->html.scroll_x;
	ay = t->valign_adjx - hw->html.scroll_y;
	ft = t->frame;
	rules = t->rules;

	if ((rules == GROUPS) && !t->col_info)
		rules = NONE;

	if (!ttopGC) {
		char dash_list[2];
		unsigned long valuemask = GCFillStyle | GCStipple;
		XGCValues values;

		values.stipple = XCreateBitmapFromData(dsp,
					 RootWindowOfScreen(scn), shadowpm_bits,
					 shadowpm_width, shadowpm_height);
		values.fill_style = FillSolid;
		ttopGC = XCreateGC(dsp, RootWindow(dsp, DefaultScreen(dsp)),
				   valuemask, &values);
		tbotGC = XCreateGC(dsp, RootWindow(dsp, DefaultScreen(dsp)),
				   valuemask, &values);
		XSetLineAttributes(dsp, ttopGC, 1, LineOnOffDash,
				   CapNotLast, JoinMiter);
		XSetForeground(dsp, ttopGC,
			       WhitePixel(dsp, DefaultScreen(dsp)));
		XSetLineAttributes(dsp, tbotGC, 0, LineOnOffDash,
				   CapNotLast, JoinMiter);
		XSetForeground(dsp, tbotGC,
			       BlackPixel(dsp, DefaultScreen(dsp)));
		dash_list[0] = '\1';
		dash_list[1] = '\1';
		XSetDashes(dsp, ttopGC, 1, dash_list, 2);
		XSetDashes(dsp, tbotGC, 1, dash_list, 2);
	}
	XSetTSOrigin(dsp, ttopGC, hw->html.scroll_x % 2, hw->html.scroll_y % 2);
	XSetTSOrigin(dsp, tbotGC, hw->html.scroll_x % 2, hw->html.scroll_y % 2);

	if (hw->html.bg_image) {
		ltopGC = ttopGC;
		lbotGC = tbotGC;
	} else {
		ltopGC = hw->manager.top_shadow_GC;
		lbotGC = hw->manager.bottom_shadow_GC;
	}
	if (t->outer_border == 1) {
		int y1;
		int y0 = 0;
		int xw = x + eptr->width - 1;

		/* Top */
		if (y >= 0) {
			if ((ft == BOX) || (ft == ABOVE) || (ft == HSIDES))
				XDrawLine(dsp, XtWindow(hw->html.view),
					  ltopGC, x, y, xw, y);
			y0 = y;
		}
		if (y + eptr->height <=  hw->html.view_height) {
			y1 = y + eptr->height - 1;
			if ((ft == BOX) || (ft == BELOW) || (ft == HSIDES))
				XDrawLine(dsp, XtWindow(hw->html.view),
					  lbotGC, x, y1, xw, y1);
		} else {
			y1 = hw->html.view_height;
		}

		/* Draw left line */
		if ((ft == BOX) || (ft == VSIDES) || (ft == LHS))
			XDrawLine(dsp, XtWindow(hw->html.view),	ltopGC,
				  x, y0, x, y1);
		if ((ft == BOX) || (ft == VSIDES) || (ft == RHS))
			XDrawLine(dsp, XtWindow(hw->html.view),	lbotGC,
				  xw, y0, xw, y1);
	} else if (t->outer_border > 1) {
		int pts = 4;
		int bx, by, bw, bh;

		if (hw->html.bg_image) {
			XSetFillStyle(dsp, ltopGC, FillStippled);
			XSetFillStyle(dsp, lbotGC, FillStippled);
		}
		/* Draw shadows. Points are numbered as follows:
		 *
		 *     5 __________________________________________
		 *      |\                                        /4
		 *      | \                                      / |
		 *      |  \                                    /  |
		 *      |   2_________________________________ 3   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |__________________________________|   |
		 *      |   1                                   \  |
		 *      |  /                                     \ |
		 *      | /_______________________________________\|
		 *       0
		 *
		 */
		if (ft == BOX) {
			pt[0].x = x;
			pt[0].y = y + eptr->height;
			pt[1].x = x + t->outer_border;
			pt[1].y = y + eptr->height - t->outer_border;
			pt[2].x = x + t->outer_border;
			pt[2].y = y + t->outer_border;
			pt[3].x = x + eptr->width - t->outer_border;
			pt[3].y = y + t->outer_border;
			pt[4].x = x + eptr->width;
			pt[4].y = y;
			pt[5].x = x;
			pt[5].y = y;
			pts = 6;
		} else if ((ft == ABOVE) || (ft == HSIDES)) {
			bx = x + t->outer_border;
			by = y;
			bw = eptr->width - (2 * t->outer_border);
			bh = t->outer_border;
		} else if ((ft == VSIDES) || (ft == LHS)) {
			bx = x;
			by = y + t->outer_border;
			bw = t->outer_border;
			bh = eptr->height - (2 * t->outer_border);
		}
		if (pts != 4) {
			XFillPolygon(dsp, XtWindow(hw->html.view),
				     ltopGC, pt, pts, Complex, CoordModeOrigin);
		} else {
			XFillRectangle(dsp, XtWindow(hw->html.view),
				       ltopGC, bx, by, bw, bh);
		}
		/* Draw shadows.  Points are numbered as follows:
		 *
		 *       __________________________________________
		 *      |\                                        /4
		 *      | \                                      / |
		 *      |  \                                    /  |
		 *      |   \_________________________________ 3   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |                                  |   |
		 *      |   |__________________________________2   |
		 *      |   1                                   \  |
		 *      |  /                                     \ |
		 *      | /_______________________________________\5
		 *       0
		 *
		 * only 2 and 5 change
		 */
		if (ft == BOX) {
			pt[2].x = x + eptr->width - t->outer_border;
			pt[2].y = y + eptr->height - t->outer_border;
			pt[5].x = x + eptr->width;
			pt[5].y = y + eptr->height;
		} else if ((ft == BELOW) || (ft == HSIDES)) {
			bx = x + t->outer_border;
			by = y + eptr->height - t->outer_border;
			bw = eptr->width - (2 * t->outer_border);
			bh = t->outer_border;
		} else if ((ft == VSIDES) || (ft == RHS)) {
			bx = x + eptr->width - t->outer_border;
			by = y + t->outer_border;
			bw = t->outer_border;
			bh = eptr->height - (2 * t->outer_border);
		}
		if (pts != 4) {
			XFillPolygon(dsp, XtWindow(hw->html.view),
				     lbotGC, pt, pts, Complex, CoordModeOrigin);
		} else {
			XFillRectangle(dsp, XtWindow(hw->html.view),
				       lbotGC, bx, by, bw, bh);
		}
		if (hw->html.bg_image) {
			XSetFillStyle(dsp, ltopGC, FillSolid);
			XSetFillStyle(dsp, lbotGC, FillSolid);
		}
	}

	if (rules == NONE)
		return;

	cells_lines = t->row_list->cells_lines;
	iseg = 0;

	for (i = 0; i < t->num_row; i++) {
		group = 0;
		for (j = 0; j < t->num_col; j++) {
			cell = cells_lines[i][j];
			cx = cell.x + ax;
			cy = cell.y + ay;
			if ((rules == GROUPS) && (cell.group > group) &&
			    (cell.back_rs == 0) && (cell.back_cs == 0)) {
				group = cell.group;
				do_group = 1;
			} else {
				do_group = 0;
			}
			cw1 = cell.width;
			if (rules != ROWS) {
 				cw1--;
			} else {
				/* Add border in */
				cx--;
				cw1++;
			}
			ch1 = cell.height;
			if ((rules != COLS) && !do_group) {
				ch1--;
			} else {
				/* Add border in */
				cy--;
				ch1++;
			}
			if (cy + ch1 < 0)
				continue;   /* Not visible : before */
			if (cy - 1 > hw->html.view_height)
				continue;   /* Not visible : after */

			if ((cell.back_rs == 0) && (cell.back_cs == 0) &&
			    (cell.has_content || (rules != ALL))) {
				XSegment *pseg = segB + iseg;

				/* Top line */
				if ((rules != COLS) && (rules != GROUPS)) {
					pseg->x1 = cx;
					pseg->y1 = cy;
					pseg->x2 = cx + cw1;
					pseg->y2 = cy;
					pseg++;
				}
				/* Left line */
				if ((ch1 > 0) &&
				    ((rules == COLS) || (rules == ALL) ||
				     ((rules == GROUPS) && do_group))) {
					pseg->x1 = cx;
					pseg->y1 = cy;
					pseg->x2 = cx;
					pseg->y2 = cy + ch1;
				}

				pseg = segT + iseg;
				/* Bottom line */
				if ((rules != COLS) && (rules != GROUPS)) {
					pseg->x1 = cx;
					pseg->y1 = cy + ch1;
					pseg->x2 = cx + cw1;
					pseg->y2 = cy + ch1;
					iseg++;
					pseg++;
				}
				/* Draw right line or complete left line */
				if ((ch1 > 0) && (rules != ROWS)) {
					if (rules != GROUPS) {
						pseg->x1 = cx + cw1;
						pseg->y1 = cy;
						pseg->x2 = cx + cw1;
						pseg->y2 = cy + ch1;
						iseg++;
					} else if (do_group) {
						/* Need complete left line */
						pseg->x1 = cx - 1;
						pseg->y1 = cy;
						pseg->x2 = cx - 1;
						pseg->y2 = cy + ch1;
						iseg++;
					}
				}
				if (iseg > (MAX_SEG - 1)) {
					XDrawSegments(dsp,
						      XtWindow(hw->html.view),
						      lbotGC, segB, iseg);
					XDrawSegments(dsp,
						      XtWindow(hw->html.view),
						      ltopGC, segT, iseg);
					iseg = 0;
				}
			}
		}
	}
	if (iseg > 0) {
		XDrawSegments(dsp, XtWindow(hw->html.view), lbotGC, segB, iseg);
		XDrawSegments(dsp, XtWindow(hw->html.view), ltopGC, segT, iseg);
	}
}

/* Display table cell backgrounds and refresh contents */
ElemInfo *CellRefresh(HTMLWidget hw, ElemInfo *eptr)
{
	ElemInfo *tptr;
	int height, celly, right_x, bot_y, left_x, top_y;
	int save_cell_has_bg = hw->html.table_cell_has_bg;
	unsigned long save_bg;
	TableInfo *t = eptr->table_data;
	CellStruct *cell = eptr->cell_data;

	celly = cell->y + t->valign_adjx;
	/* Skip it if cell had no contents, start of refresh is on cell's
	 * bottom border or end of refresh is on cell's top border */
	if (!cell->has_content ||
	    (hw->html.redisplay_y > (celly + cell->height - t->borders)) ||
	    ((hw->html.redisplay_y + hw->html.redisplay_height) <
	     (celly + t->borders)))
		return(eptr);

	hw->html.table_cell_has_bg = 1;
	/* Do cell back ground color */
	if (hw->html.redisplay_y > (celly + t->borders)) {
		top_y = hw->html.redisplay_y - hw->html.scroll_y;
		if ((hw->html.redisplay_y + hw->html.redisplay_height) >
		    (celly + cell->height - t->borders)) {
			height = cell->height - t->borders -
				 (hw->html.redisplay_y - celly);
		} else {
			height = hw->html.redisplay_height;
		}
	} else {
		top_y = celly + t->borders - hw->html.scroll_y;
		if ((hw->html.redisplay_y + hw->html.redisplay_height) >
		    (celly + cell->height - t->borders)) {
			height = cell->height - (2 * t->borders);
		} else {
			height = hw->html.redisplay_height -
				 ((celly + t->borders) - hw->html.redisplay_y);
		}
	}
	if (hw->html.cur_fg != eptr->bg) {
		XSetForeground(hw->html.dsp, hw->html.drawGC, eptr->bg);
		hw->html.cur_fg = eptr->bg;
	}
	XFillRectangle(hw->html.dsp, XtWindow(hw->html.view), hw->html.drawGC,
		       cell->x + t->align_adjx + t->borders - hw->html.scroll_x,
		       top_y, cell->width - t->borders, height);

	save_bg = hw->core.background_pixel;
	hw->core.background_pixel = eptr->bg;
	/* Refresh rest of cell */
	top_y = hw->html.redisplay_y;
	bot_y = top_y + hw->html.redisplay_height;
	left_x = hw->html.scroll_x;
	right_x = left_x + hw->html.view_width;
	tptr = eptr;
	eptr = eptr->next;
	while (eptr && (tptr != cell->end_elem)) {
		/* Skip if out range and not CR (needed to reset underlining) */
		if (((eptr->y + eptr->height) < top_y) || (eptr->y > bot_y) ||
		    (eptr->x > right_x) ||
		    (((eptr->x + eptr->width) < left_x) &&
		     (eptr->type != E_CR))) {
			tptr = eptr;
			eptr = eptr->next;
			continue;
		}
#ifndef DISABLE_TRACE
		if (refreshTrace)
			fprintf(stderr,
				"Calling RefreshElement in CellRefresh\n");
#endif
		eptr = RefreshElement(hw, eptr);
		tptr = eptr;
		eptr = eptr->next;
	}
	hw->core.background_pixel = save_bg;
	hw->html.table_cell_has_bg = save_cell_has_bg;

	return(tptr);
}

#ifndef DISABLE_TRACE
static void TableDump(TableInfo *t)
{
	register int x, y;

	fprintf(stderr, "---- Table dump ----\n");
	fprintf(stderr, "Table width = %d, Border width = %d\n",
		t->width, t->borders);
	fprintf(stderr, "Table height = %d, Columns = %d, Rows = %d\n",
		t->height, t->num_col, t->num_row);
	fprintf(stderr, "-------------------------------------------\n");
	for (y = 0; y < t->num_row; y++) {
	    for (x = 0; x < t->num_col; x++) {
		fprintf(stderr, "W=%d,H=%d ",
			t->row_list->cells_lines[y][x].width,
			t->row_list->cells_lines[y][x].height);
		if (t->row_list->cells_lines[y][x].cell_type == M_TD_CELL_PAD) {
		    fprintf(stderr, "Pad ");
		} else if (t->row_list->cells_lines[y][x].cell_type ==
			   M_TD_CELL_FREE) {
		    fprintf(stderr, "Free ");
		}
		fprintf(stderr, "| ");
	    }
	    fprintf(stderr, "\n----------------------------------\n");
	}
}
#endif
