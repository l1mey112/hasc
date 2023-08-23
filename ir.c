#include <stdio.h>

#include "hasc.h"
#include "ir.h"
#include "shared.h"
#include "type.h"

void _hproc_ast_dump(hcc_ctx_t *ctx, u32 cfg) {
	printf("expr");
}

void hproc_ast_dump(hcc_ctx_t *ctx, u32 cfg) {
	_hproc_ast_dump(ctx, cfg);
	printf("\n");
}

void hproc_cfg_dump(hcc_ctx_t *ctx, u32 cfg) {
	hcfg_node_t *p = hcc_cfg_node(ctx, cfg);

	printf("%u:\n", cfg);

	u32 ast_p = p->ast_begin;
	while (ast_p != (u32)-1) {
		hast_node_t *p = hcc_ast_node(ctx, ast_p);
		printf("\t");
		hproc_ast_dump(ctx, ast_p);
		ast_p = p->next;
	}

	if (p->ast_cond != (u32)-1) {
		printf("if (");
		_hproc_ast_dump(ctx, cfg);
		printf(") goto %u else goto %u\n", p->node_true, p->node_false);

		hproc_cfg_dump(ctx, p->node_true);
		hproc_cfg_dump(ctx, p->node_false);
	}
}

void hproc_dump(hcc_ctx_t *ctx, hproc_t *proc) {
	printf("%.*s: ", (int)proc->fn_name.len, proc->fn_name.p);
	htable_type_dump(ctx, proc->fn_type);

	if (proc->cfg_begin == (u32)-1) {
		return;
	}

	hproc_cfg_dump(ctx, proc->cfg_begin);
}