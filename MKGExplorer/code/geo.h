#pragma once
#include <vector>
#include <string>
struct geo_header {
	char header[4]; //1.0v
	int  archiveSize;
	int  files;
	char pad[20] = {};
};

struct geo_entry {
	short unk;
	short models;
	int   offset; // - headersize
	char pad[24] = {};
};

struct geo_image_entry {
	short width;
	short height;
	char pad[28];
};

struct geometry_info {
	int		unk;
	short	unk2;
	short   vertexes;
	char pad[24] = {};
};

struct vector3d {
	float x;
	float y;
	float z;
};


struct v { float x, y, z; };
struct uv { float u, v; };
struct vn { float norm[3]; };
struct face { int face[3]; };

struct group_info {
	int belong;
	std::string name;

	std::vector<v> Verts;
	std::vector<uv> Maps;
	std::vector<vn> Normals;
	std::vector<face> Faces;

};