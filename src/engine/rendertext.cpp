#include "shared/cube.h"
//#include "engine/engine.h"
#include "engine/texture.h"
#include "engine/rendergl.h"
#include "engine/engine/font.h"

void draw_textf(const char *fstr, float left, float top, ...)
{
	defvformatcubestr(str, top, fstr);
	draw_text(str, left, top);
}

const matrix4x3 *textmatrix = NULL;
float textscale = 1;

static float draw_char(Texture *&tex, int c, float x, float y, float scale)
{
	font::charinfo &info = font::CurrentFont()->chars[c-font::CurrentFont()->charoffset];
	if(tex != font::CurrentFont()->texs[info.tex])
	{
		xtraverts += gle::end();
		tex = font::CurrentFont()->texs[info.tex];
        glCheckError(glBindTexture(GL_TEXTURE_2D, tex->id));
	}

	x *= textscale;
	y *= textscale;
	scale *= textscale;

	float x1 = x + scale*info.offsetx,
		  y1 = y + scale*info.offsety,
		  x2 = x + scale*(info.offsetx + info.w),
		  y2 = y + scale*(info.offsety + info.h),
		  tx1 = info.x / tex->xs,
		  ty1 = info.y / tex->ys,
		  tx2 = (info.x + info.w) / tex->xs,
		  ty2 = (info.y + info.h) / tex->ys;

	if(textmatrix)
	{
		gle::attrib(textmatrix->transform(vec2(x1, y1))); gle::attribf(tx1, ty1);
		gle::attrib(textmatrix->transform(vec2(x2, y1))); gle::attribf(tx2, ty1);
		gle::attrib(textmatrix->transform(vec2(x2, y2))); gle::attribf(tx2, ty2);
		gle::attrib(textmatrix->transform(vec2(x1, y2))); gle::attribf(tx1, ty2);
	}
	else
	{
		gle::attribf(x1, y1); gle::attribf(tx1, ty1);
		gle::attribf(x2, y1); gle::attribf(tx2, ty1);
		gle::attribf(x2, y2); gle::attribf(tx2, ty2);
		gle::attribf(x1, y2); gle::attribf(tx1, ty2);
	}

	return scale*info.advance;
}

VARP(textbright, 0, 85, 100);

//stack[sp] is current color index
static void text_color(char c, char *stack, int size, int &sp, bvec color, int a)
{
	if(c=='s') // save color
	{
		c = stack[sp];
		if(sp<size-1) stack[++sp] = c;
	}
	else
	{
		xtraverts += gle::end();
		if(c=='r') { if(sp > 0) --sp; c = stack[sp]; } // restore color
		else stack[sp] = c;
		switch(c)
		{
			case '0': color = bvec( 64, 255, 128); break;   // green: player talk
			case '1': color = bvec( 96, 160, 255); break;   // blue: "echo" command
			case '2': color = bvec(255, 192,  64); break;   // yellow: gameplay messages
			case '3': color = bvec(255,  64,  64); break;   // red: important errors
			case '4': color = bvec(128, 128, 128); break;   // gray
			case '5': color = bvec(192,  64, 192); break;   // magenta
			case '6': color = bvec(255, 128,   0); break;   // orange
			case '7': color = bvec(255, 255, 255); break;   // white
			case '8': color = bvec( 80, 207, 229); break;   // "SchizoMania Blue"
			case '9': color = bvec(160, 240, 120); break;
			default: gle::color(color, a); return;          // provided color: everything else
		}
		if(textbright != 100) color.scale(textbright, 100);
		gle::color(color, a);
	}
}

#define TEXTSKELETON \
	float y = 0, x = 0, scale = font::CurrentFont()->scale/float(font::CurrentFont()->defaulth);\
	int i;\
	for(i = 0; str[i]; i++)\
	{\
		TEXTINDEX(i)\
		int c = uchar(str[i]);\
		if(c=='\t')      { x = TEXTTAB(x); TEXTWHITE(i) }\
		else if(c==' ')  { x += scale*font::CurrentFont()->defaultw; TEXTWHITE(i) }\
		else if(c=='\n') { TEXTLINE(i) x = 0; y += FONTH; }\
		else if(c=='\f') { if(str[i+1]) { i++; TEXTCOLOR(i) }}\
		else if(in_range(c-font::CurrentFont()->charoffset, font::CurrentFont()->chars))\
		{\
			float cw = scale*font::CurrentFont()->chars[c-font::CurrentFont()->charoffset].advance;\
			if(cw <= 0) continue;\
			if(maxwidth >= 0)\
			{\
				int j = i;\
				float w = cw;\
				for(; str[i+1]; i++)\
				{\
					int c = uchar(str[i+1]);\
					if(c=='\f') { if(str[i+2]) i++; continue; }\
					if(!in_range(c-font::CurrentFont()->charoffset, font::CurrentFont()->chars)) break;\
					float cw = scale*font::CurrentFont()->chars[c-font::CurrentFont()->charoffset].advance;\
					if(cw <= 0 || w + cw > maxwidth) break;\
					w += cw;\
				}\
				if(x + w > maxwidth && x > 0) { (void)j; TEXTLINE(j-1) x = 0; y += FONTH; }\
				TEXTWORD\
			}\
			else { TEXTCHAR(i) }\
		}\
	}

//all the chars are guaranteed to be either drawable or color commands
#define TEXTWORDSKELETON \
				for(; j <= i; j++)\
				{\
					TEXTINDEX(j)\
					int c = uchar(str[j]);\
					if(c=='\f') { if(str[j+1]) { j++; TEXTCOLOR(j) }}\
					else { float cw = scale*font::CurrentFont()->chars[c-font::CurrentFont()->charoffset].advance; TEXTCHAR(j) }\
				}

#define TEXTEND(cursor) if(cursor >= i) { do { TEXTINDEX(cursor); } while(0); }

int text_visible(const char *str, float hitx, float hity, int maxwidth)
{
	#define TEXTINDEX(idx)
	#define TEXTWHITE(idx) if(y+FONTH > hity && x >= hitx) return idx;
	#define TEXTLINE(idx) if(y+FONTH > hity) return idx;
	#define TEXTCOLOR(idx)
	#define TEXTCHAR(idx) x += cw; TEXTWHITE(idx)
	#define TEXTWORD TEXTWORDSKELETON
	TEXTSKELETON
	#undef TEXTINDEX
	#undef TEXTWHITE
	#undef TEXTLINE
	#undef TEXTCOLOR
	#undef TEXTCHAR
	#undef TEXTWORD
	return i;
}

//inverse of text_visible
void text_posf(const char *str, int cursor, float &cx, float &cy, int maxwidth)
{
	#define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; break; }
	#define TEXTWHITE(idx)
	#define TEXTLINE(idx)
	#define TEXTCOLOR(idx)
	#define TEXTCHAR(idx) x += cw;
	#define TEXTWORD TEXTWORDSKELETON if(i >= cursor) break;
	cx = cy = 0;
	TEXTSKELETON
	TEXTEND(cursor)
	#undef TEXTINDEX
	#undef TEXTWHITE
	#undef TEXTLINE
	#undef TEXTCOLOR
	#undef TEXTCHAR
	#undef TEXTWORD
}

void text_boundsf(const char *str, float &width, float &height, int maxwidth)
{
	#define TEXTINDEX(idx)
	#define TEXTWHITE(idx)
	#define TEXTLINE(idx) if(x > width) width = x;
	#define TEXTCOLOR(idx)
	#define TEXTCHAR(idx) x += cw;
	#define TEXTWORD x += w;
	width = 0;
	TEXTSKELETON
	height = y + FONTH;
	TEXTLINE(_)
	#undef TEXTINDEX
	#undef TEXTWHITE
	#undef TEXTLINE
	#undef TEXTCOLOR
	#undef TEXTCHAR
	#undef TEXTWORD
}

Shader *textshader = NULL;

void draw_text(const char *str, float left, float top, int r, int g, int b, int a, int cursor, int maxwidth)
{
	#define TEXTINDEX(idx) if(idx == cursor) { cx = x; cy = y; }
	#define TEXTWHITE(idx)
	#define TEXTLINE(idx)
	#define TEXTCOLOR(idx) if(usecolor) text_color(str[idx], colorstack, sizeof(colorstack), colorpos, color, a);
	#define TEXTCHAR(idx) draw_char(tex, c, left+x, top+y, scale); x += cw;
	#define TEXTWORD TEXTWORDSKELETON
	char colorstack[10];
	colorstack[0] = '\0'; //indicate user color
	bvec color(r, g, b);
	if(textbright != 100) color.scale(textbright, 100);
	int colorpos = 0;
	float cx = -FONTW, cy = 0;
	bool usecolor = true;
	if(a < 0) { usecolor = false; a = -a; }
	Texture *tex = font::CurrentFont()->texs[0];
	(textshader ? textshader : hudtextshader)->set();
	LOCALPARAMF(textparams, font::CurrentFont()->bordermin, font::CurrentFont()->bordermax, font::CurrentFont()->outlinemin, font::CurrentFont()->outlinemax);
    glCheckError(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glCheckError(glBindTexture(GL_TEXTURE_2D, tex->id));
	gle::color(color, a);
	gle::defvertex(textmatrix ? 3 : 2);
	gle::deftexcoord0();
	gle::begin(GL_QUADS);
	TEXTSKELETON
	TEXTEND(cursor)
	xtraverts += gle::end();
	if(cursor >= 0 && (totalmillis/250)&1)
	{
		gle::color(color, a);
		if(maxwidth >= 0 && cx >= maxwidth && cx > 0) { cx = 0; cy += FONTH; }
		draw_char(tex, '_', left+cx, top+cy, scale);
		xtraverts += gle::end();
	}
	#undef TEXTINDEX
	#undef TEXTWHITE
	#undef TEXTLINE
	#undef TEXTCOLOR
	#undef TEXTCHAR
	#undef TEXTWORD
}



// >>>>>>>>>> SCRIPTBIND >>>>>>>>>>>>>> //
#if 0
#include "/Users/micha/dev/ScMaMike/src/build/binding/..+engine+rendertext.binding.cpp"
#endif
// <<<<<<<<<< SCRIPTBIND <<<<<<<<<<<<<< //
