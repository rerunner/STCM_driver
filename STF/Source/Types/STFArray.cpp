class STFIntArray
	{
	protected:
		uint32 size;
		uint32 numElements;
		int32 * data;
	public:
		STFArray(uint32 initialSize)
			{
			ASSERT(initialSize > 0);
			
			size = initialSize;
			numElements = 0;

			data = new int32[initialSize];
			};

		~STFArray()
			{
			if (data)
				delete[] data;
			};

		STFResult Add(int32 data)
			{
			if (numElements == size)
				{
				newData = new int32[numElements * 2];
				memcpy(newData, data, numElements);
				size *= 2;				
				};
			
			data[numElements++] = data;
			};

		STFResult SetAt(unit32 at, int32 data)
			{
			if (at >= size)
				{		
				size = at;
				
				newData = new int32[at];
				memcpy(newData, data, numElements);
				numElements = at;
				}

			data[at] = data;

			STFRES_RAISE_OK;
			};

		STFResult ElementAt(unit32 at, int32 & data) const
			{
			if (at < numElements)
				{
				data = data[at];
				};
			else
				{
				STFRES_RAISE(STFRES_OBJECT_NOT_FOUND);
				}

			STFRES_RAISE_OK;
			};

		uint32 Size() const
			{
			return numElements;
			}		
	};

