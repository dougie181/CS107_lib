#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "streamtokenizer.h"
#include "html-utils.h"
#include <assert.h>

#define UNICODE_MAX 0x10FFFFul
#define DEBUG_HTML 0

int decode_html_entities_utf8(char *dest, const char *src);

static const char *const NAMED_ENTITIES[][2] = {
  { "AElig;", "Æ" },
  { "Aacute;", "Á" },
  { "Acirc;", "Â" },
  { "Agrave;", "À" },
  { "Alpha;", "Α" },
  { "Aring;", "Å" },
  { "Atilde;", "Ã" },
  { "Auml;", "Ä" },
  { "Beta;", "Β" },
  { "Ccedil;", "Ç" },
  { "Chi;", "Χ" },
  { "Dagger;", "‡" },
  { "Delta;", "Δ" },
  { "ETH;", "Ð" },
  { "Eacute;", "É" },
  { "Ecirc;", "Ê" },
  { "Egrave;", "È" },
  { "Epsilon;", "Ε" },
  { "Eta;", "Η" },
  { "Euml;", "Ë" },
  { "Gamma;", "Γ" },
  { "Iacute;", "Í" },
  { "Icirc;", "Î" },
  { "Igrave;", "Ì" },
  { "Iota;", "Ι" },
  { "Iuml;", "Ï" },
  { "Kappa;", "Κ" },
  { "Lambda;", "Λ" },
  { "Mu;", "Μ" },
  { "Ntilde;", "Ñ" },
  { "Nu;", "Ν" },
  { "OElig;", "Œ" },
  { "Oacute;", "Ó" },
  { "Ocirc;", "Ô" },
  { "Ograve;", "Ò" },
  { "Omega;", "Ω" },
  { "Omicron;", "Ο" },
  { "Oslash;", "Ø" },
  { "Otilde;", "Õ" },
  { "Ouml;", "Ö" },
  { "Phi;", "Φ" },
  { "Pi;", "Π" },
  { "Prime;", "″" },
  { "Psi;", "Ψ" },
  { "Rho;", "Ρ" },
  { "Scaron;", "Š" },
  { "Sigma;", "Σ" },
  { "THORN;", "Þ" },
  { "Tau;", "Τ" },
  { "Theta;", "Θ" },
  { "Uacute;", "Ú" },
  { "Ucirc;", "Û" },
  { "Ugrave;", "Ù" },
  { "Upsilon;", "Υ" },
  { "Uuml;", "Ü" },
  { "Xi;", "Ξ" },
  { "Yacute;", "Ý" },
  { "Yuml;", "Ÿ" },
  { "Zeta;", "Ζ" },
  { "aacute;", "á" },
  { "acirc;", "â" },
  { "acute;", "´" },
  { "aelig;", "æ" },
  { "agrave;", "à" },
  { "alefsym;", "ℵ" },
  { "alpha;", "α" },
  { "amp;", "&" },
  { "and;", "∧" },
  { "ang;", "∠" },
  { "apos;", "'" },
  { "aring;", "å" },
  { "asymp;", "≈" },
  { "atilde;", "ã" },
  { "auml;", "ä" },
  { "bdquo;", "„" },
  { "beta;", "β" },
  { "brvbar;", "¦" },
  { "bull;", "•" },
  { "cap;", "∩" },
  { "ccedil;", "ç" },
  { "cedil;", "¸" },
  { "cent;", "¢" },
  { "chi;", "χ" },
  { "circ;", "ˆ" },
  { "clubs;", "♣" },
  { "cong;", "≅" },
  { "copy;", "©" },
  { "crarr;", "↵" },
  { "cup;", "∪" },
  { "curren;", "¤" },
  { "dArr;", "⇓" },
  { "dagger;", "†" },
  { "darr;", "↓" },
  { "deg;", "°" },
  { "delta;", "δ" },
  { "diams;", "♦" },
  { "divide;", "÷" },
  { "eacute;", "é" },
  { "ecirc;", "ê" },
  { "egrave;", "è" },
  { "empty;", "∅" },
  { "emsp;", "\xE2\x80\x83" },
  { "ensp;", "\xE2\x80\x82" },
  { "epsilon;", "ε" },
  { "equiv;", "≡" },
  { "eta;", "η" },
  { "eth;", "ð" },
  { "euml;", "ë" },
  { "euro;", "€" },
  { "exist;", "∃" },
  { "fnof;", "ƒ" },
  { "forall;", "∀" },
  { "frac12;", "½" },
  { "frac14;", "¼" },
  { "frac34;", "¾" },
  { "frasl;", "⁄" },
  { "gamma;", "γ" },
  { "ge;", "≥" },
  { "gt;", ">" },
  { "hArr;", "⇔" },
  { "harr;", "↔" },
  { "hearts;", "♥" },
  { "hellip;", "…" },
  { "iacute;", "í" },
  { "icirc;", "î" },
  { "iexcl;", "¡" },
  { "igrave;", "ì" },
  { "image;", "ℑ" },
  { "infin;", "∞" },
  { "int;", "∫" },
  { "iota;", "ι" },
  { "iquest;", "¿" },
  { "isin;", "∈" },
  { "iuml;", "ï" },
  { "kappa;", "κ" },
  { "lArr;", "⇐" },
  { "lambda;", "λ" },
  { "lang;", "〈" },
  { "laquo;", "«" },
  { "larr;", "←" },
  { "lceil;", "⌈" },
  { "ldquo;", "“" },
  { "le;", "≤" },
  { "lfloor;", "⌊" },
  { "lowast;", "∗" },
  { "loz;", "◊" },
  { "lrm;", "\xE2\x80\x8E" },
  { "lsaquo;", "‹" },
  { "lsquo;", "‘" },
  { "lt;", "<" },
  { "macr;", "¯" },
  { "mdash;", "—" },
  { "micro;", "µ" },
  { "middot;", "·" },
  { "minus;", "−" },
  { "mu;", "μ" },
  { "nabla;", "∇" },
  { "nbsp;", "\xC2\xA0" },
  { "ndash;", "–" },
  { "ne;", "≠" },
  { "ni;", "∋" },
  { "not;", "¬" },
  { "notin;", "∉" },
  { "nsub;", "⊄" },
  { "ntilde;", "ñ" },
  { "nu;", "ν" },
  { "oacute;", "ó" },
  { "ocirc;", "ô" },
  { "oelig;", "œ" },
  { "ograve;", "ò" },
  { "oline;", "‾" },
  { "omega;", "ω" },
  { "omicron;", "ο" },
  { "oplus;", "⊕" },
  { "or;", "∨" },
  { "ordf;", "ª" },
  { "ordm;", "º" },
  { "oslash;", "ø" },
  { "otilde;", "õ" },
  { "otimes;", "⊗" },
  { "ouml;", "ö" },
  { "para;", "¶" },
  { "part;", "∂" },
  { "permil;", "‰" },
  { "perp;", "⊥" },
  { "phi;", "φ" },
  { "pi;", "π" },
  { "piv;", "ϖ" },
  { "plusmn;", "±" },
  { "pound;", "£" },
  { "prime;", "′" },
  { "prod;", "∏" },
  { "prop;", "∝" },
  { "psi;", "ψ" },
  { "quot;", "\"" },
  { "rArr;", "⇒" },
  { "radic;", "√" },
  { "rang;", "〉" },
  { "raquo;", "»" },
  { "rarr;", "→" },
  { "rceil;", "⌉" },
  { "rdquo;", "”" },
  { "real;", "ℜ" },
  { "reg;", "®" },
  { "rfloor;", "⌋" },
  { "rho;", "ρ" },
  { "rlm;", "\xE2\x80\x8F" },
  { "rsaquo;", "›" },
  { "rsquo;", "’" },
  { "sbquo;", "‚" },
  { "scaron;", "š" },
  { "sdot;", "⋅" },
  { "sect;", "§" },
  { "shy;", "\xC2\xAD" },
  { "sigma;", "σ" },
  { "sigmaf;", "ς" },
  { "sim;", "∼" },
  { "spades;", "♠" },
  { "sub;", "⊂" },
  { "sube;", "⊆" },
  { "sum;", "∑" },
  { "sup1;", "¹" },
  { "sup2;", "²" },
  { "sup3;", "³" },
  { "sup;", "⊃" },
  { "supe;", "⊇" },
  { "szlig;", "ß" },
  { "tau;", "τ" },
  { "there4;", "∴" },
  { "theta;", "θ" },
  { "thetasym;", "ϑ" },
  { "thinsp;", "\xE2\x80\x89" },
  { "thorn;", "þ" },
  { "tilde;", "˜" },
  { "times;", "×" },
  { "trade;", "™" },
  { "uArr;", "⇑" },
  { "uacute;", "ú" },
  { "uarr;", "↑" },
  { "ucirc;", "û" },
  { "ugrave;", "ù" },
  { "uml;", "¨" },
  { "upsih;", "ϒ" },
  { "upsilon;", "υ" },
  { "uuml;", "ü" },
  { "weierp;", "℘" },
  { "xi;", "ξ" },
  { "yacute;", "ý" },
  { "yen;", "¥" },
  { "yuml;", "ÿ" },
  { "zeta;", "ζ" },
  { "zwj;", "\xE2\x80\x8D" },
  { "zwnj;", "\xE2\x80\x8C" }
};

static int cmp(const void *key, const void *value)
{
  return strncmp((const char *)key, *(const char *const *)value, strlen(*(const char *const *)value));
}

static const char *get_named_entity(const char *name)
{
  const char *const *entity = (const char *const *)bsearch(name, NAMED_ENTITIES, sizeof NAMED_ENTITIES / sizeof *NAMED_ENTITIES, sizeof *NAMED_ENTITIES, cmp);

  return entity ? entity[1] : NULL;
}

static size_t putc_utf8(unsigned long cp, char *buffer)
{
  unsigned char *bytes = (unsigned char *)buffer;

  if(cp <= 0x007Ful)
  {
    bytes[0] = (unsigned char)cp;
    return 1;
  }

  if(cp <= 0x07FFul)
  {
    bytes[1] = (unsigned char)((2 << 6) | (cp & 0x3F));
    bytes[0] = (unsigned char)((6 << 5) | (cp >> 6));
    return 2;
  }

  if(cp <= 0xFFFFul)
  {
    bytes[2] = (unsigned char)(( 2 << 6) | ( cp       & 0x3F));
    bytes[1] = (unsigned char)(( 2 << 6) | ((cp >> 6) & 0x3F));
    bytes[0] = (unsigned char)((14 << 4) |  (cp >> 12));
    return 3;
  }

  if(cp <= 0x10FFFFul)
  {
    bytes[3] = (unsigned char)(( 2 << 6) | ( cp        & 0x3F));
    bytes[2] = (unsigned char)(( 2 << 6) | ((cp >>  6) & 0x3F));
    bytes[1] = (unsigned char)(( 2 << 6) | ((cp >> 12) & 0x3F));
    bytes[0] = (unsigned char)((30 << 3) |  (cp >> 18));
    return 4;
  }

  return 0;
}

static bool parse_entity(const char *current, char **to, const char **from)
{
  const char *end = strchr(current, ';');
  if(!end) return 0;

  if(current[1] == '#')
  {
    char *tail = NULL;
    int errno_save = errno;
    bool hex = current[2] == 'x' || current[2] == 'X';

    errno = 0;
    unsigned long cp = strtoul(
    current + (hex ? 3 : 2), &tail, hex ? 16 : 10);

    bool fail = errno || tail != end || cp > UNICODE_MAX;
    errno = errno_save;
    if(fail) return 0;

    *to += putc_utf8(cp, *to);
    *from = end + 1;

    return 1;
  } else {
    const char *entity = get_named_entity(&current[1]);
    if(!entity) return 0;

    size_t len = strlen(entity);
    memcpy(*to, entity, len);

    *to += len;
    *from = end + 1;

    return 1;
  }
}

int decode_html_entities_utf8(char *dest, const char *src)
{
  int result = 0;

  if(!src) src = dest;

  char *to = dest;
  const char *from = src;

  for(const char *current; (current = strchr(from, '&'));)
  {
    memmove(to, from, (size_t)(current - from));
    to += current - from;

    if(parse_entity(current, &to, &from))
      continue;

    from = current;
    *to++ = *from++;
  }

  size_t remaining = strlen(from);

  memmove(to, from, remaining);
  to += remaining;
  *to = 0;
  
  result = (to - dest);
  return result;
}

void RemoveEscapeCharacters(char text[])
{
  assert(text != NULL);

  char *source = strdup(text);
  char *dest = strdup(text);
 
  assert(source !=NULL);
  assert(dest != NULL);

  int length = decode_html_entities_utf8(dest, source);
  
  char *ptr = text; 
  for (int i = 0; i < length; i++) {
    ptr = (char *)ptr + i;
    ptr = (char *)dest + i;
  }
  ptr = ptr + 1;
  *ptr = '\0';

  if (DEBUG_HTML) printf("removingESC: converted text %s to %s\n",source,text);
  
  free(source);
  free(dest);
}

bool extractCDATA(streamtokenizer *st, char htmlBuffer[], int htmlBufferLength)
{
  char CDATAString[] = "<![CDATA[";
  assert(htmlBuffer != NULL);
  assert(htmlBufferLength >= 2);

  // asume that stream is pointing to a <!CDATA[
  
  // copy the buffer to a temporary location
  char *tempString = strdup(htmlBuffer);
  assert(tempString != NULL);

  char *startPos = strstr(htmlBuffer, CDATAString) + strlen(CDATAString);
  char *endPos = strstr(htmlBuffer,"]");
  int length = endPos - startPos;

  if (DEBUG_HTML) printf("htmlBuffer length = %d\n",length);

  char *strPtr;
  strPtr = htmlBuffer;
  for (int i = 0; i < length; i++)
    htmlBuffer[i] = tempString[i + strlen(CDATAString)];

  htmlBuffer[length] = '\0'; 
  free(tempString);

  return true;
}

bool GetNextTag(streamtokenizer *st, char htmlBuffer[], int htmlBufferLength)
{
  assert(htmlBuffer != NULL && htmlBufferLength > 2);

  int endTagFound = 0;

  int next = STSkipUntil(st, "<");
  if (next != EOF) {
    // ok, so this is not perfect as we may be inside a script or comment
    // tag and have encountered a "<" character that does not represent the
    // start of an actual tag OR we will find a ">" character that does not 
    // represent the end of a html tag...

    // place in the buffer all chars up to and including the > char
    int i = 0;
    while (!endTagFound && i < (htmlBufferLength - 1) ) {
      next = fgetc(st->infile);

      htmlBuffer[i++] = next;

      if (next == EOF) return false;

      if (next == '>') endTagFound = 1;
    }

    // terminate the string
    htmlBuffer[i] = '\0';

    if (!endTagFound && DEBUG_HTML) printf(">>>GetNextTag: %s\n",htmlBuffer);
    
    return true;

  } else {
    return false;
  }
}

void SkipIrrelevantContent(streamtokenizer *st)
{
  // need to read everything betweeen <aaa xxx bbb>
   
  char buffer[1024];
  int bufferLength = sizeof(buffer);
  bool foundToken = false;
  int styleFound = 0;
  int commentFound = 0;
  int scriptFound = 0;

  // look at all text up until the next > character 
  // we do not want to advance the stream just yet until we work out if irrelevant 
  // content occuring
  // assume we are currently on a "<" character
  foundToken = STLookAtNextToken(st, buffer, bufferLength, ">");

  if (DEBUG_HTML) printf("     buffer = %s\n",buffer);

  if (foundToken) {
    // ok, just need to check if it is a comment ie starts with <!-- xxx -->
    if (strncmp(buffer, "!--", 3) == 0) 
    {
      if (DEBUG_HTML) printf("found a comment... now skip until end of comment\n");
      // remove up until the closing comment (ie -->)

      while (!commentFound) {
 
        // now check if this ends in a -->
        char *bufferPtr = (char *)buffer + strlen(buffer) - 2;
        if (strncmp(bufferPtr,"--",2) == 0) {
          commentFound = 1; 
        } else {
          // continue getting token up until next >
          // advance the stream until we get the next > character
          STNextTokenUsingDifferentDelimiters(st, buffer, bufferLength,">");
        }
      }
    } 
    if (strncmp(buffer, "script", 6) == 0) {
      if (DEBUG_HTML) printf("found a script... now skip until find a </script>\n");
      while (!scriptFound) {
      
        // now check if we find a closing </script>
        STNextTokenUsingDifferentDelimiters(st, buffer, bufferLength,">");
      
        if (strlen(buffer) > 7) {
          char *bufferPtr = (char *)buffer + strlen(buffer) - 8;
          if (strncmp(bufferPtr,"</script",8) == 0)
            scriptFound = 1;
        }
      }
    } 
    if (strncmp(buffer, "style", 5) == 0) {
      if (DEBUG_HTML) printf("found a style... now skip until find a </style>\n");
      while (!styleFound) {
      
        // now check if we find a closing </style>
        STNextTokenUsingDifferentDelimiters(st, buffer, bufferLength,">");
  
        if (strlen(buffer) > 6) {
          char *bufferPtr = (char *)buffer + strlen(buffer) - 7;
          if (strncmp(bufferPtr, "</style", 7) == 0)
            styleFound = 1;
        }
      }
    }
    if ((styleFound == 1 || scriptFound == 1 || commentFound == 1)) {
      return;
    } else {
      STSkipUntil(st, ">");
    }
  }
}
