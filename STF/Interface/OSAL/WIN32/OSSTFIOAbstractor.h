
#ifndef OSSTFIOABSTRACTOR_H
#define OSSTFIOABSTRACTOR_H

class OSSTFIOAbstractor
	{
	public:
      OSSTFIOAbstractor(){};

		void Printf(const char* format, va_list list)
			{
			vprintf(format, list);
			}

		STFString GetEnvironmentVariable(const char *var, const char* defaultString = NULL)
			{
			char * envString = getenv(var);

			if (envString)
				return STFString(envString);
			else
				if (defaultString)
					return STFString(defaultString);
				else
					return STFString("");
			}
	};

#endif //of OSSTFIOABSTRACTOR_H
