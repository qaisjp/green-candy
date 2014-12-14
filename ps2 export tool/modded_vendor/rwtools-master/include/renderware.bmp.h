// Bitmap software rendering includes.
struct Bitmap
{
    inline Bitmap( void )
    {
        this->width = 0;
        this->height = 0;
        this->depth = 0;
        this->rasterFormat = RASTER_8888;
        this->texels = NULL;
        this->dataSize = 0;
    }

    inline Bitmap( uint32 depth, eRasterFormat theFormat )
    {
        this->width = 0;
        this->height = 0;
        this->depth = depth;
        this->rasterFormat = theFormat;
        this->texels = NULL;
        this->dataSize = 0;
    }

    inline Bitmap( const Bitmap& right )
    {
        this->width = right.width;
        this->height = right.height;
        this->depth = right.depth;
        this->rasterFormat = right.rasterFormat;

        // Copy texels.
        void *origTexels = right.texels;
        void *newTexels = NULL;

        if ( origTexels )
        {
            newTexels = new uint8[ right.dataSize ];

            memcpy( newTexels, origTexels, right.dataSize );
        }

        this->texels = newTexels;
        this->dataSize = right.dataSize;
    }

    inline ~Bitmap( void )
    {
        // If we have texel data, deallocate it.
        if ( void *ourTexels = this->texels )
        {
            delete [] ourTexels;
        }
    }

    inline static uint32 getRasterFormatDepth( eRasterFormat format )
    {
        uint32 theDepth = 0;

        if ( format == RASTER_8888 || format == RASTER_888 || format == RASTER_32 )
        {
            theDepth = 32;
        }
        else if ( format == RASTER_1555 || format == RASTER_565 || format == RASTER_4444 || format == RASTER_555 || format == RASTER_16 )
        {
            theDepth = 16;
        }
        else if ( format == RASTER_24 )
        {
            theDepth = 24;
        }
        else if ( format == RASTER_LUM8 )
        {
            theDepth = 8;
        }

        return theDepth;
    }

    inline static uint32 getRasterImageDataSize( uint32 width, uint32 height, uint32 depth )
    {
        uint32 imageItemCount = ( width * height );
        uint32 bitsPerPixel = depth;

        return ( imageItemCount * depth / 8 );
    }

    inline void setImageData( void *theTexels, eRasterFormat theFormat, uint32 depth, uint32 width, uint32 height, uint32 dataSize )
    {
        this->width = width;
        this->height = height;
        this->depth = depth;
        this->rasterFormat = theFormat;

        // Deallocate texels if we already have some.
        if ( void *origTexels = this->texels )
        {
            delete [] origTexels;

            this->texels = NULL;
        }

        // Copy the texels.
        if ( dataSize != 0 )
        {
            void *newTexels = new uint8[ dataSize ];

            memcpy( newTexels, theTexels, dataSize );

            this->texels = newTexels;
        }
        this->dataSize = dataSize;
    }

    inline void setImageData( void *theTexels, eRasterFormat theFormat, uint32 depth, uint32 width, uint32 height )
    {
        uint32 dataSize = getRasterImageDataSize( width, height, depth );

        setImageData( theTexels, theFormat, depth, width, height, dataSize );
    }

    void setSize( uint32 width, uint32 height );

    inline void getSize( uint32& width, uint32& height ) const
    {
        width = this->width;
        height = this->height;
    }

    inline uint32 getDepth( void ) const
    {
        return this->depth;
    }

    inline eRasterFormat getFormat( void ) const
    {
        return this->rasterFormat;
    }

    inline uint32 getDataSize( void ) const
    {
        return this->dataSize;
    }
    
    inline void* copyPixelData( void ) const
    {
        void *newPixels = NULL;
        void *ourPixels = this->texels;

        if ( ourPixels )
        {
            newPixels = new uint8[ this->dataSize ];

            memcpy( newPixels, ourPixels, this->dataSize );
        }

        return newPixels;
    }

    enum eBlendMode
    {
        BLEND_MODULATE,
        BLEND_MODULATE_ADD,
        BLEND_ADDITIVE
    };

    enum eShadeMode
    {
        SHADE_SRCALPHA,
        SHADE_INVSRCALPHA
    };

    struct sourceColorPipeline abstract
    {
        virtual uint32 getWidth( void ) const = 0;
        virtual uint32 getHeight( void ) const = 0;

        virtual void fetchcolor( uint32 colorIndex, double& red, double& green, double& blue, double& alpha ) = 0;
    };

    void draw(
        sourceColorPipeline& colorSource,
        uint32 offX, uint32 offY, uint32 drawWidth, uint32 drawHeight,
        eShadeMode srcChannel, eShadeMode dstChannel, eBlendMode blendMode
    );

    void drawBitmap(
        const Bitmap& theBitmap, uint32 offX, uint32 offY, uint32 drawWidth, uint32 drawHeight,
        eShadeMode srcChannel, eShadeMode dstChannel, eBlendMode blendMode
    );

private:
    uint32 width, height;
    uint32 depth;
    eRasterFormat rasterFormat;
    void *texels;
    uint32 dataSize;
};