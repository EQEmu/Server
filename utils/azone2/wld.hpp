#ifndef __OPENEQ_WLD__
#define __OPENEQ_WLD__

#include <stdio.h>

#include "global.hpp"

#include "file_loader.hpp"

#include "wld_structs.hpp"

#include "3d.hpp"

#define FRAGMENT(name) \
this->frags[i] = new name(this, buffer, frag->size, frag->nameRef);

class TexRef {
public:
  Texture **tex;
  int tex_count;
};

class Fragment {
public:
  Fragment() {}
  virtual ~Fragment() {}

  int type, name;
  void *frag;
};

class WLDLoader : public FileLoader {
public:
  WLDLoader();
  ~WLDLoader();

  virtual int Open(char *base_path, char *zone_name, Archive *archive);
  virtual int Close();

  int fragcount;
  Fragment **frags;
  BSP_Node *tree;

  uchar *sHash;

  char old;

  WLDLoader *obj_loader;
  WLDLoader *plac_loader;

  char clear_plac;
};

#define FRAG_CLASS(name) \
class name : public Fragment { \
public: \
  name(WLDLoader *wld, uchar *buf, int len, int frag_name); \
  ~name() {}; \
}

#define FRAG_CONSTRUCTOR(name) \
name::name(WLDLoader *wld, uchar *buf, int len, int frag_name)

#define FRAG_DECONSTRUCTOR(name) \
name::~name()

FRAG_CLASS(Data03);
FRAG_CLASS(Data04);
FRAG_CLASS(Data05);
FRAG_CLASS(Data15);
FRAG_CLASS(Data1B);
FRAG_CLASS(Data1C);
FRAG_CLASS(Data21);
FRAG_CLASS(Data22);
FRAG_CLASS(Data29);
FRAG_CLASS(Data28);
FRAG_CLASS(Data30);
FRAG_CLASS(Data31);
FRAG_CLASS(Data36);

void DoubleLinkBSP(BSP_Node *tree, long node_number, long parent);
long BSPMarkRegion(BSP_Node *tree, long node_number, long region, int region_type);

#endif
