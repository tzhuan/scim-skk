#ifdef HAVE_GETTEXT
#include <libintl.h>
#define _(String) dgettext(GETTEXT_PACKAGE,String)
#define N_(String) (String)
#else
#define _(String) (String)
#define N_(String) (String)
#define bindtextdomain(Package,Directory)
#define textdomain(domain)
#define bind_textdomain_codeset(domain,codeset)
#endif

