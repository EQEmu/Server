#include <perlbind/perlbind.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

EXTERN_C
{
  void boot_DynaLoader(pTHX_ CV* cv);
  static void xs_init(pTHX)
  {
    newXS(const_cast<char*>("DynaLoader::boot_DynaLoader"), boot_DynaLoader, const_cast<char*>(__FILE__));
  }
}

namespace perlbind {

interpreter::interpreter()
  : m_is_owner(true)
{
  const char* argv[] = { "", "-ew", "0", nullptr };
  constexpr int argc = (sizeof(argv) / sizeof(*argv)) - 1;
  init(argc, argv);
}

interpreter::interpreter(int argc, const char** argv)
  : m_is_owner(true)
{
  init(argc, argv);
}

void interpreter::init(int argc, const char** argv)
{
  char** argvs = const_cast<char**>(argv);
  char** env = { nullptr };

  // PERL_SYS_INIT3 and PERL_SYS_TERM should only be called once per program
  PERL_SYS_INIT3(&argc, &argvs, &env);

  my_perl = perl_alloc();
  PERL_SET_CONTEXT(my_perl);
  PL_perl_destruct_level = 1;
  perl_construct(my_perl);
  perl_parse(my_perl, xs_init, argc, argvs, nullptr);

  perl_run(my_perl);
}

interpreter::~interpreter()
{
  if (m_is_owner)
  {
    PL_perl_destruct_level = 1;
    perl_destruct(my_perl);
    perl_free(my_perl);

    PERL_SYS_TERM();
  }
}

void interpreter::load_script(std::string packagename, std::string filename)
{
  struct stat st{};
  if (stat(filename.c_str(), &st) != 0)
  {
    throw std::runtime_error("Unable to read perl file '" + filename + "'");
  }

  std::ifstream ifs(filename);
  std::stringstream buffer;
  buffer << "package " << packagename << "; " << ifs.rdbuf();

  try
  {
    eval(buffer.str().c_str());
  }
  catch (std::exception& e)
  {
    throw std::runtime_error("Error loading script '" + filename + "':\n " + e.what());
  }
}

void interpreter::eval(const char* str)
{
  SV* sv = eval_pv(str, 0);
  if (sv == &PL_sv_undef)
  {
    SV* err = get_sv("@", 0);
    if (err && err->sv_u.svu_pv[0])
    {
      throw std::runtime_error(err->sv_u.svu_pv);
    }

    throw std::runtime_error("unknown error in eval()");
  }
}

} // namespace perlbind
