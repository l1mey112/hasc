#include "all.h"

#include <setjmp.h>
#include <stdarg.h>
#include "stb_ds.h"

void err_with_pos(loc_t loc, const char *fmt, ...) {
	char buf[256];
	
	va_list args;
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	file_entry_t *file = &file_entries[loc.file];

	snprintf(err_diag.err_string, sizeof(err_diag.err_string), "%s:%u:%u: %s", file->fp, loc.line_nr + 1, loc.col + 1, buf);
	longjmp(err_diag.unwind, 1);
}

void err_without_pos(const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vsnprintf(err_diag.err_string, sizeof(err_diag.err_string), fmt, args);
	va_end(args);
	longjmp(err_diag.unwind, 1);
}

// used for memory that lasts forever, simple bump ptr allocator
static u8 scratch_buf[8192];
static u8 *scratch_p = scratch_buf;

u8 *alloc_scratch(size_t size) {
	assert(scratch_p + size <= scratch_buf + sizeof(scratch_buf));
	
	u8 *p = scratch_p;
	scratch_p += size;
	return p;
}

void alloc_reset(u8 *p) {
	assert(p >= scratch_buf && p <= scratch_buf + sizeof(scratch_buf));
	scratch_p = p;
}

// TODO: insert types? if the value is `void` no need to assign
static void _dump_inst(hir_proc_t *proc, hir_inst_t *inst) {
	/* if (inst->type == TYPE_VOID) {
		eprintf("     = ");
	} else {
		eprintf("%%%-3u = ", inst->id);
	} */
	eprintf("%%%u = ", inst->id);
	switch (inst->kind) {
		case HIR_ARG:
			eprintf("arg(l%u:%s)\n", inst->d_local.local, sv_from(proc->locals[inst->d_local.local].name));
			break;
		case HIR_LOCAL:
			eprintf("local(l%u:%s)\n", inst->d_local.local, sv_from(proc->locals[inst->d_local.local].name));
			break;
		case HIR_LOCAL_GET:
			eprintf("local_get(%%%u)\n", proc->locals[inst->d_local.local].inst);
			break;
		case HIR_LOCAL_SET:
			eprintf("local_set(%%%u, %%%u)\n", proc->locals[inst->d_local.local].inst, inst->d_local_set.src);
			break;
		case HIR_SYM_GET:
			eprintf("sym_get(%s)\n", sv_from(inst->d_sym.lit));
			break;
		case HIR_INTEGER_LITERAL:
			if (inst->d_literal.negate) {
				eprintf("-%s\n", sv_from(inst->d_literal.lit));
			} else {
				eprintf("%s\n", sv_from(inst->d_literal.lit));
			}
			break;
		case HIR_INFIX:
			eprintf("%%%u %s %%%u\n", inst->d_infix.lhs, tok_literal_representation(inst->d_infix.op), inst->d_infix.rhs);
			break;
		case HIR_PREFIX:
			eprintf("%s %%%u\n", tok_literal_representation(inst->d_prefix.op), inst->d_prefix.val);
			break;
		case HIR_RETURN:
			eprintf("return");
			for (u32 i = 0; i < inst->d_return.retc; i++) {
				eprintf(" %%%u", inst->d_return.retl[i]);
				if (i + 1 < inst->d_return.retc) {
					eprintf(",");
				}
			}
			eprintf("\n");
			break;
		default:
			assert_not_reached();
	}
}

void dump_proc(hir_proc_t *proc) {
	u8 *sc = alloc_scratch(0);

	eprintf("%s: %s\n", sv_from(proc->name), table_type_dbg_str(proc->type));

	for (hir_rblock_t i = 0; i < arrlenu(proc->blocks); i++) {
		hir_block_t *block = &proc->blocks[i];
		eprintf("%u:\n", i);
		for (hir_rinst_t j = block->first; j < block->first + block->len; j++) {
			hir_inst_t *inst = &proc->insts[j];

			eprintf("\t");
			_dump_inst(proc, inst);
		}
	}

	alloc_reset(sc);
}