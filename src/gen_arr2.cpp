#include <math.h>
#include <stdlib.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////
// Tlsh.java code to generate the bit_pairs_diff_table in tlsh_util.cpp

int result[256][256];

void generateTable()
{
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                int x = i, y = j, d, diff = 0;
                d = abs(x % 4 - y % 4); diff += (d == 3 ? 6 : d); 
                x /= 4; y /= 4;
                d = abs(x % 4 - y % 4); diff += (d == 3 ? 6 : d); 
                x /= 4; y /= 4;
                d = abs(x % 4 - y % 4); diff += (d == 3 ? 6 : d); 
                x /= 4; y /= 4;
                d = abs(x % 4 - y % 4); diff += (d == 3 ? 6 : d); 
                result[i][j] = diff;
            }   
        }   
}

/////////////////////////////////////////////////////////////////////////////
// Jon Oliver's functions to generate bit_pairs_diff_table

static int pairbit_diff(int pairb, int opairb)
{
	int diff = abs(pairb - opairb);
	if (diff <= 1)
		return(diff);
	else if (diff == 2)
		return(2);
	return(6);
}

int byte_diff(unsigned char bv, unsigned char obv)
{
	int h1	= (unsigned char) bv  / 16;
	int oh1	= (unsigned char) obv / 16;
	int h2	= (unsigned char) bv  % 16;
	int oh2	= (unsigned char) obv % 16;
	int p1	= h1 / 4;
	int op1	= oh1 / 4;
	int p2	= h1 % 4;
	int op2	= oh1 % 4;
	int p3	= h2 / 4;
	int op3	= oh2 / 4;
	int p4	= h2 % 4;
	int op4	= oh2 % 4;
	int diff = 0;
	diff = diff + pairbit_diff(p1, op1);
	diff = diff + pairbit_diff(p2, op2);
	diff = diff + pairbit_diff(p3, op3);
	diff = diff + pairbit_diff(p4, op4);
	return(diff);
}

/////////////////////////////////////////////////////////////////////////////
// main() function to verify Tlsh.java and Jon's implementations are equalivant,
//        and to output the static unsigned char bit_pairs_diff_table in
//        tlsh_util.cpp.
int main()
{
int x;
int y;
	generateTable();
	for (x=0; x<256; x++) {
		printf("{\n");
		for (y=0; y<256; y++) {
			int z = byte_diff((unsigned char) x, (unsigned char) y);
			if (z != result[x][y]) {
				printf("\nWARNING x=%d y=%d z=%d	nuno=%d\n", x, y, z, result[x][y]);
				return -1;
			}
			printf("%d", z);
			if (y < 255)
				printf(", ");
			if (y % 16 == 15)
				printf("\n");
		}
		printf("}");
		if (x < 255)
			printf(",");
		printf("\n");
	}
	printf("};");
}
