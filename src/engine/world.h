#pragma once

enum                            // hardcoded texture numbers
{
    DEFAULT_SKY = 0,
    DEFAULT_GEOM,
    NUMDEFAULTSLOTS
};

#define OCTAVERSION 33

struct octaheader
{
    char magic[4];              // "OCTA"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int worldsize;
    int numents;
    int numpvs;
    int lightmaps;
    int blendmap;
    int numvars;
    int numvslots;
};

#define MAPVERSION 2            // bump if map format changes, see worldio.cpp

struct mapheader
{
    char magic[4];              // "SCMA"
    int version;                // any >8bit quantity is little endian
    int headersize;             // sizeof(header)
    int worldsize;
    int numents;
    int numpvs;
    int blendmap;
    int numvars;
    int numvslots;
};

#define WATER_AMPLITUDE 0.4f
#define WATER_OFFSET 1.1f

enum
{
    MATSURF_NOT_VISIBLE = 0,
    MATSURF_VISIBLE,
    MATSURF_EDIT_ONLY
};

#define TEX_SCALE 16.0f

struct vertex { vec pos; bvec4 norm; vec tc; bvec4 tangent; };

template <class ET>
ET* getentitybytype(int searchStartIndex = 0)
{
	const auto &ents = entities::getents();
	for(int i = searchStartIndex; i < ents.length(); i++)
	{
		auto e = dynamic_cast<ET*>(ents[i]);

		if (e) return e;
	}

	return nullptr;
}

entities::classes::CoreEntity *new_game_entity(bool local, const vec &o, int &idx, const char *strclass = "");
