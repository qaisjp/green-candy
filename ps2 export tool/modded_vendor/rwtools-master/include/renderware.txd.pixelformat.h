namespace PixelFormat
{   
    inline uint32 coord2index(uint32 x, uint32 y, uint32 stride)
    {
        return ( y * stride + x );
    }

    struct palette4bit
    {
        struct data
        {
            uint8 fp_index : 4;
            uint8 sp_index : 4;
        };

        typedef uint8 trav_t;

        static inline uint32 sizeitems( size_t itemCount )
        {
            return ALIGN_SIZE( itemCount, (size_t)2 ) / 2;
        }

        static inline palette4bit* allocate( size_t itemCount )
        {
            return (palette4bit*)new uint8[ sizeitems( itemCount ) ];
        }

        inline data* findpalette(uint32 index) const
        {
            uint32 dataIndex = ( index / 2 );

            return ( (data*)this + dataIndex );
        }

        inline void setvalue(uint32 index, trav_t palette)
        {
            uint32 indexSelector = ( index % 2 );

            data *theData = findpalette(index);

            if ( indexSelector == 0 )
            {
                theData->fp_index = palette;
            }
            else if ( indexSelector == 1 )
            {
                theData->sp_index = palette;
            }
        }

        inline void getvalue(uint32 index, trav_t& palette) const
        {
            uint32 indexSelector = ( index % 2 );

            const data *theData = findpalette(index);

            if ( indexSelector == 0 )
            {
                palette = theData->fp_index;
            }
            else if ( indexSelector == 1 )
            {
                palette = theData->sp_index;
            }
        }
    };

    struct palette8bit
    {
        struct data
        {
            uint8 index;
        };

        typedef uint8 trav_t;

        static uint32 sizeitems( size_t itemCount )
        {
            return ( itemCount * sizeof(uint8) );
        }

        static inline palette8bit* allocate( size_t itemCount )
        {
            return (palette8bit*)new uint8[ sizeitems( itemCount ) ];
        }

        inline data* findpalette(uint32 index) const
        {
            return ( (data*)this + index );
        }

        inline void setvalue(uint32 index, trav_t palette)
        {
            data *theData = findpalette(index);

            theData->index = palette;
        }

        inline void getvalue(uint32 index, trav_t& palette) const
        {
            const data *theData = findpalette(index);

            palette = theData->index;
        }
    };

    template <typename pixelstruct>
    struct typedcolor
    {
        typedef pixelstruct trav_t;

        static uint32 sizeitems( size_t itemCount )
        {
            return ( itemCount * sizeof( itemCount ) );
        }

        static inline typedcolor* allocate( size_t itemCount )
        {
            return (typedcolor*)new uint8[ sizeitems( itemCount ) ];
        }

        inline pixelstruct* finddata(uint32 index) const
        {
            return ( (pixelstruct*)this + index );
        }

        inline void setvalue(uint32 index, trav_t palette)
        {
            pixelstruct *theData = finddata(index);

            *theData = palette;
        }

        inline void getvalue(uint32 index, trav_t& palette) const
        {
            const pixelstruct *theData = finddata(index);

            palette = *theData;
        }
    };

    struct pixeldata32bit
    {
        uint8 red;
        uint8 green;
        uint8 blue;
        uint8 alpha;
    };

    template <typename pixelstruct>
    struct texeltemplate
    {
        inline void setcolor(uint32 index, uint8 red, uint8 green, uint8 blue, uint8 alpha)
        {
            pixelstruct *pixelData = ( (pixelstruct*)this + index );

            pixelData->red = red;
            pixelData->green = green;
            pixelData->blue = blue;
            pixelData->alpha = alpha;
        }

        inline void getcolor(uint32 index, uint8& red, uint8& green, uint8& blue, uint8& alpha)
        {
            const pixelstruct *pixelData = ( (const pixelstruct*)this + index );

            red = pixelData->red;
            green = pixelData->green;
            blue = pixelData->blue;
            alpha = pixelData->alpha;
        }
    };
};