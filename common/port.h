#pragma once

#if (WIN32)
#define SCANF sscanf_s
#define STRCMPNOCASE _strnicmp
#else
#define SCANF sscanf
#define STRCMPNOCASE strncasecmp
#endif