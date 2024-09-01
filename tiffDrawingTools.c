/*
This library generates tiff format images without any compression;
to create a image buffer structure:
	screen *sp = sopen(int width, int height);
to write data to a file:
	writeFile(char *filename, screen *structurePointer);
to free all memory used by the structure:
 	sclose(screen *structurePointer);

NOTE: This program works for LITTLE ENDIAN images. If you want to make an image that opens on a big endian machine, go to line 175 and change the "II" to "MM"
NOTE: the drawing functions come in several formats: CONVENIENT, and STANDARD. 
	STANDARD format pixel ranges start at 0 and end at the the width or length of the image.
 	CONVENIENT format pixel ranges from -1.0 to 1.0
*/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define SHORT 3
#define LONG 4
#define RATIONAL 5


struct screen {   // image data entity
	uint8_t *dat;
	int w, l, datmax;
	char *name;
	uint8_t r, g, b;
} typedef screen;


void drawPixel(screen *s, double x, double y) {  // paint one pixel. format: CONVENIENT
	if(y > 1 || x > 1 || x < -1 || y < -1)
		return;
	int b = (s->w)*3*((int)((y+1)*(s->l/2))) + 3*((int)((x+1)*(s->w/2)));
	s->dat[b]   = s->r;
	s->dat[b+1] = s->g;
	s->dat[b+2] = s->b;
}

void setPixel(screen *s, int x, int y) { // paint one pixel. format: STANDARD
	int b = y*(s->w)*3 + x*3;
	s->dat[b]   = s->r;
	s->dat[b+1] = s->g;
	s->dat[b+2] = s->b;
}
void setPixelWithColor(screen *s, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
	int d = y*(s->w)*3 + x*3;
	s->dat[d] = r;
	s->dat[d+1] = g;
	s->dat[d+2] = b;
}

void drawCircle(screen *s, double px, double py, double r) { // paint a circle. format CONVENIENT
	int fx = ((px+1)*(s->w/2.0));
	int fy = ((py+1)*(s->l/2.0));
	int ir = r*(s->w/2.0);
	int x1 = fx - ir;
	int x2 = fx + ir;
	int y1 = fy - ir;
	int y2 = fy + ir;
	int x,y,b;
	for(y = y1; y <= y2; y++) {
		for(x = x1; x <= x2 && y >= 0 && y < s->l; x++) {
			if(x >= 0 && x < s->w && (b = 3*(s->w)*y + 3*x) < s->datmax - 3 && b >= 0 && (y-fy)*(y-fy) + (x-fx)*(x-fx) < ir*ir) {
				s->dat[b++] = s->r;
				s->dat[b++] = s->g;
				s->dat[b]   = s->b;
			}
		}
	}
	
}
void setColor(screen *s, int r, int g, int b) {   // change the current color of the brush
	s->r = r;
	s->g = g;
	s->b = b;
}

void drawLine2d(screen *s, int px1, int py1, int px2, int py2) { // format: STANDARD
	int x1 = px1 < px2 ? (px1) : (px2);
	int x2 = px1 < px2 ? (px2) : (px1);
	int y1 = px1 < px2 ? (py1) : (py2);
	int y2 = px1 < px2 ? (py2) : (py1);

	int x, y;
	int b;
	for(x = x1; x <= x2; x++) {
		y = ((double)(y1-y2)/((double)(x1-x2)) * (double)(x-x1) + (double)y1);
		if(x >= 0 && x < s->w && (b = 3*(s->w)*y + 3*x) < s->datmax - 3 && b >= 0) {
			s->dat[b++] = s->r;
			s->dat[b++] = s->g;
			s->dat[b]   = s->b;
		}
	}
	if(y2 < y1) { // if the Ys are in reverse order of magnitude, reverse all the coords.
		y = y2;
		y2 = y1;
		y1 = y;
		x = x2;
		x2 = x1;
		x1 = x;
	}
	for(y = y1; y <= y2; y++) {
		x = ((double)(x1-x2)/((double)(y1-y2)) * (double)(y-y1) + (double)x1);
		if(x >= 0 && x < s->w && (b = 3*(s->w)*y + 3*x) < s->datmax - 3 && b >= 0) {
			s->dat[b++] = s->r;
			s->dat[b++] = s->g;
			s->dat[b]   = s->b;
		}
	}
}
void drawLine(screen *s, double px1, double py1, double px2, double py2) { // format: CONVENIENT
	drawLine2d(s, (px1+1)*(s->w/2.0),(py1+1)*(s->l/2.0),(px2+1)*(s->w/2.0),(py2+1)*(s->l/2.0));
}

void drawSquare(screen *s, double px1, double py1, double px2, double py2) { // format CONVENIENT
	int x1 = px1 < px2 ? ((px1+1)*(s->w/2.0)) : ((px2+1)*(s->w/2.0));
	int x2 = px1 < px2 ? ((px2+1)*(s->w/2.0)) : ((px1+1)*(s->w/2.0));
	int y1 = py1 < py2 ? ((py1+1)*(s->l/2.0)) : ((py2+1)*(s->l/2.0));
	int y2 = py1 < py2 ? ((py2+1)*(s->l/2.0)) : ((py1+1)*(s->l/2.0));
	int x,y,b;
	for(y = y1; y <= y2; y++) {
		for(x = x1; x <= x2; x++) {
			if((b = 3*(s->w)*y + 3*x) < s->datmax - 3) {
				s->dat[b++] = s->r;
				s->dat[b++] = s->g;
				s->dat[b]   = s->b;
			}
		}
	}
}


void init(int wid, int len,screen *s) {
	s->datmax = wid*len*3;
	s->dat = malloc(s->datmax);
	memset(s->dat,0,wid*len*3);
	s->w = wid;
	s->l = len;
	s->r = s->g = s->b = 255;
}

screen *sopen(int wid, int len) {
	screen *out = malloc(sizeof(screen));
	out->datmax = wid*len*sizeof(uint8_t)*3;
	out->dat = malloc(out->datmax);
	memset(out->dat,0,out->datmax);
	out->w = wid;
	out->l = len;
	out->r = out->g = out->b = 255;
	return out;
}

void unInit(screen *s) {
	free(s->dat);
	free(s);
}

void sclose(screen *s) {
	unInit(s);
}


void write_IFD(uint16_t tag, uint16_t field, uint32_t vals, uint32_t offset, FILE *fp) {
	fwrite(&tag,2,1,fp);
	fwrite(&field,2,1,fp);
	fwrite(&vals,4,1,fp);
	fwrite(&offset,4,1,fp);
}

void writeHeader(FILE *fp, int w, int l) {
	uint16_t bytes_2 = 42;
	uint32_t bytes_4 = 8;
	// header
	fwrite("II",1,2,fp);         // 0-1 "II" for LITTLE ENDIAN, and "MM" for BIG ENDIAN
	fwrite(&bytes_2,2,1,fp);  // 2-3
	fwrite(&bytes_4,4,1,fp);  // 4-7 offset of first ifd (8)
	
	bytes_2 = 12;
	fwrite(&bytes_2,2,1,fp);                  // 8-9
	write_IFD(256,SHORT,1,(w),fp);         // IMAGE WIDTH
	write_IFD(257,SHORT,1,(l),fp);         // IMAGE LENGTH
	write_IFD(258,SHORT,3,(174),fp);          // BITS PER SAMPLE
	write_IFD(259,SHORT,1,(1),fp);            // COMPRESSION (None)
	write_IFD(262,SHORT,1,(2),fp);            // PHOTOMETRIC INTERPRETATION (black is zero)
	write_IFD(273,SHORT,1,(180),fp);          // STRIP OFFSET (offset of each strip)
	write_IFD(277,SHORT,1,(3),fp);            //SAMPLES PER PIXEL
	write_IFD(278,SHORT,1,(l),fp);         // ROWS PER STRIP
	write_IFD(279,LONG,1,((w)*(l)),fp); // STRIP BYTE COUNT
	write_IFD(282,RATIONAL,1,158,fp);         // X RESOLUTION
	write_IFD(283,RATIONAL,1,166,fp);         // Y RESOLUTION
	write_IFD(296,SHORT,1,(1),fp);            // RESOLUTION UNIT
	
	bytes_4 = 0;
	fwrite(&bytes_4,4,1,fp);
	
	bytes_4 = 1;
	fwrite(&bytes_4,4,1,fp);
	fwrite(&bytes_4,4,1,fp);
	fwrite(&bytes_4,4,1,fp);
	fwrite(&bytes_4,4,1,fp); // X and Y resolution
	// 162
	bytes_2 = 8;
	fwrite(&bytes_2,2,1,fp);
	fwrite(&bytes_2,2,1,fp);
	fwrite(&bytes_2,2,1,fp);
}

void writeFile(char *name, screen *s) {
	FILE *fp = fopen(name,"w");
	writeHeader(fp,s->w,s->l);

	fwrite(s->dat,(s->w) * (s->l) * 3,1,fp);
	
	fclose(fp);
}

