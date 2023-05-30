#include <stdio.h>
#include "process.h"
#include "audit.h"

static FILE *fdb, *fde;

void audit_open() {
	fdb = fopen("blocks.csv", "w");
	fde = fopen("segments.csv", "w");
}

void audit_block(Block *block) {
	for (unsigned i = 0; i < block->count; i++)
		fprintf(fdb, "[%d], %f, %f, %f, %f\n", i,
				block->sample_a[i], block->sample_b[i],
				block->sample_c[i], block->sample_d[i]);
	fprintf(fdb, "\n");
}

void audit_segment(Levels *levels) {
	fprintf(fde, "# block, LAFmax, LAFmin, LApeak, LAE, LAeq\n");

	for (unsigned i = 0; i < levels->block_number; i++) {
		fprintf(fde, "[%d], %f, %f, %f, %f, %f\n", i,
			levels->LAFmax[i], levels->LAFmin[i], levels->LApeak[i],
				levels->LAEsum[i], levels->LAEsum[i]);

	}
}

void audit_close() {
	fclose(fdb);
	fclose(fde);
}
