#include <StdInc.h>

#include <bitset>
#define _USE_MATH_DEFINES
#include <math.h>
#include <map>
#include <algorithm>
#include <cmath>

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"

#include <libimagequant.h>

namespace rw
{

struct palettizer
{
    struct texel_t
    {
        inline texel_t( void )
        {
            red = 0;
            green = 0;
            blue = 0;
            alpha = 0;
            usageCount = 0;
        }

        uint8 red;
        uint8 green;
        uint8 blue;
        uint8 alpha;
        uint32 usageCount;
    };

    static inline double color2double(uint8 color)
    {
        return ( (double)color / 255.0f );
    }

    static inline uint8 double2color(double num)
    {
        return (uint8)( num * 255.0f );
    }

    static inline double colorbrightness(uint8 red, uint8 green, uint8 blue, uint8 alpha)
    {
        return
            std::max(color2double(red), std::max(color2double(green), color2double(blue))) * color2double(alpha);
    }

    typedef std::vector <texel_t> texelContainer_t;

    static const uint32 maxtexellinearelimination = 25000;

    texelContainer_t texelElimData;

    uint32 alphaZeroTexelCount;

    inline palettizer( void )
    {
        // Initialize characteristics.
        this->hasBrightest = false;
        this->hasDimmest = false;

        texelElimData.reserve( maxtexellinearelimination );

        this->alphaZeroTexelCount = 0;
    }

    inline ~palettizer( void )
    {
        return;
    }

    // Some characteristics about the underlying texels.
    texel_t brightestColor;
    double brightest;
    bool hasBrightest;

    texel_t dimmestColor;
    double dimmest;
    bool hasDimmest;

    template <typename resultType>
    struct resultCache
    {
        struct indexTuple
        {
            uint32 i, n;

            inline indexTuple(uint32 i, uint32 n)
            {
                this->i = i;
                this->n = n;
            }

            inline bool operator == ( const indexTuple& right ) const
            {
                return ( this->i == right.i && this.n == right.n || this->i == right.n && this->n == right.i );
            }

            inline bool operator < ( const indexTuple& right ) const
            {
                return false;
            }
        };

        typedef std::map <indexTuple, resultType> resultMap_t;

        resultMap_t results;

        inline bool getCachedResult(uint32 i, uint32 n, resultType& result)
        {
            resultMap_t::const_iterator forward_iter = results.find( indexTuple(i, n) );

            if ( forward_iter != results.end() )
            {
                result = forward_iter->second;
                return true;
            }

            return false;
        }

        inline void putCachedResult(uint32 i, uint32 n, const resultType& result)
        {
            results[ indexTuple(i, n) ] = result;
        }

        inline void eliminate(uint32 n)
        {
            resultMap_t::iterator iter = results.begin();

            while (iter != results.end())
            {
                const indexTuple& tuple = iter->first;

                if (tuple.n == n || tuple.i == n)
                {
                    iter = results.erase(iter);
                }
                else
                {
                    iter++;
                }
            }
        }
    };

    template <typename criteriaType>
    inline bool findmostsimilarpixel(const criteriaType& parser, typename criteriaType::result_t& theResult, uint32 first, uint32& second)
    {
        size_t numTexels = texelElimData.size();

        uint32 smallest;
        typename criteriaType::result_t smallestResult;
        bool hasSmallest = false;

        {
            const texel_t& firstComparand = texelElimData.at( first );

            for (size_t i = 0; i < numTexels; i++)
            {
                if (i != first)
                {
                    const texel_t& secondComparand = texelElimData.at( i );

                    typename criteriaType::result_t thisResult = parser.getCriteria(firstComparand, secondComparand);

                    if (!hasSmallest || parser.isSmaller(thisResult, smallestResult))
                    {
                        smallest = i;
                        smallestResult = thisResult;

                        if (!hasSmallest)
                        {
                            hasSmallest = true;
                        }
                    }
                }
            }
        }

        if (hasSmallest)
        {
            second = smallest;
            theResult = smallestResult;
        }

        return hasSmallest;
    }

    template <typename criteriaType>
    inline bool findpixelundercriteria(const criteriaType& parser, typename criteriaType::result_t& theResult, uint32& first, uint32& second)
    {
        size_t numTexels = texelElimData.size();

        uint32 smallest_first, smallest_second;
        typename criteriaType::result_t smallest;
        bool hasSmallest = false;

        for (size_t n = 0; n < numTexels; n++)
        {
            const texel_t& firstComparand = texelElimData.at( n );

            for (size_t i = 0; i < numTexels; i++)
            {
                if (i != n)
                {
                    typename criteriaType::result_t thisResult;

                    const texel_t& secondComparand = texelElimData.at( i );

                    thisResult = parser.getCriteria(firstComparand, secondComparand);

                    if (!hasSmallest || parser.isSmaller(thisResult, smallest))
                    {
                        smallest_first = n;
                        smallest_second = i;
                        smallest = thisResult;

                        if (!hasSmallest)
                        {
                            hasSmallest = true;
                        }
                    }
                }
            }
        }

        if (hasSmallest)
        {
            first = smallest_first;
            second = smallest_second;
            theResult = smallest;
        }

        return hasSmallest;
    }

    struct brightnessCriteria
    {
        bool calculateBrightest;

        inline brightnessCriteria(bool brightest)
        {
            this->calculateBrightest = brightest;
        }

        typedef double result_t;

        inline result_t getCriteria(const texel_t& firstComparand, const texel_t& secondComparand) const
        {
            double firstBrightness =
                colorbrightness(firstComparand.red, firstComparand.green, firstComparand.blue, firstComparand.alpha);

            double secondBrightness =
                colorbrightness(secondComparand.red, secondComparand.green, secondComparand.blue, secondComparand.alpha);

            return fabs( std::max(firstBrightness, secondBrightness) - std::min(firstBrightness, secondBrightness) );
        }

        inline bool isSmaller(result_t first, result_t second) const
        {
            return ( calculateBrightest ) ? (first > second) : (first < second);
        }
    };

    struct colordiffCriteria
    {
        struct result_t
        {
            double angleDiff;
            double distance;
            double alphaDist;

            inline double getCriterion( void ) const
            {
                return ( distance + alphaDist * 10 );
            }
        };

        struct vec4_t
        {
            double x, y, z, w;
        };

        static inline double rad2deg(double rad)
        {
            return rad * 180 / M_PI;
        }

        static inline double deg2rad(double deg)
        {
            return deg / 180 * M_PI;
        }

        inline static void RGB2HSV(const texel_t& color, double& hueOut, double& saturationOut, double& valueOut)
        {
            // Transform all color components into doubles.
            double firstRed = color2double(color.red);
            double firstGreen = color2double(color.green);
            double firstBlue = color2double(color.blue);

            double low = std::min(firstRed, std::min(firstGreen, firstBlue));
            double brightness = std::max(firstRed, std::max(firstGreen, firstBlue));

            double delta = brightness - low;

            double hueColor = 0;

            double saturation = 0.0f;

            double colorX = 0;
            double colorY = 0;

            if (delta != 0)
            {
                saturation = delta / brightness;

                double deltar = (brightness - firstRed);
                double deltag = (brightness - firstGreen);
                double deltab = (brightness - firstBlue);

                // Calculate the hue angle.
                double hueStart, hueModDelta;

                if (firstRed == brightness)
                {
                    hueStart = (0.0/3.0);
                    hueModDelta = (deltab - deltag);
                }
                else if (firstGreen == brightness)
                {
                    hueStart = (1.0/3.0);
                    hueModDelta = (deltar - deltab);
                }
                else if (firstBlue == brightness)
                {
                    hueStart = (2.0/3.0);
                    hueModDelta = (deltag - deltar);
                }

                // Adjust the HUE mod.
                double hueMod = ((hueModDelta / 6.0) + delta / 2) / brightness;

                // Calculate the hue region.
                hueColor = hueStart + hueMod;

                // Convert it to an angle.
                if ( hueColor < 0 )
                {
                    hueColor = hueColor + 1;
                }
                else if ( hueColor > 1 )
                {
                    hueColor = hueColor - 1;
                }

                hueColor = hueColor * M_PI * 2;
            }

            // Give the hue to the runtime.
            hueOut = hueColor;

            saturationOut = saturation;

            valueOut = brightness;
        }

        inline static double saturatecolor(double theColor)
        {
            return std::min(1.0, std::max(0.0, theColor));
        }

        inline static void HSV2RGB(double hue, double saturation, double value, texel_t& colorOut)
        {
            // Convert the HUE to RGB.
            double hueFloatingPoint = ( hue / M_PI / 2 );

            double delta = value * saturation;

            double redColor;
            double greenColor;
            double blueColor;

            if (delta == 0)
            {
                redColor = 0;
                greenColor = 0;
                blueColor = 0;
            }
            else
            {
                double hueStartPlusMod = hueFloatingPoint - delta/2/value;

                double hueRegion = hueStartPlusMod;

                // Normalize the hue.
                while (hueRegion < 0.0)
                {
                    hueRegion += 1.0;
                }

                while (hueRegion >= 1.0)
                {
                    hueRegion -= 1.0;
                }

                double hueStart = 0.0f;

                if (hueRegion >= 0.0 && hueRegion <= (1.0/6.0))
                {
                    // brightest: red, green, blue
                    redColor = value;
                    blueColor = value - delta;

                    hueStart = (0.0/3.0);
                }
                else if (hueRegion <= (2.0/6.0))
                {
                    // brightest: green, red, blue
                    greenColor = value;
                    blueColor = value - delta;

                    hueStart = (1.0/3.0);
                }
                else if (hueRegion <= (3.0/6.0))
                {
                    // brightest: green, blue, red
                    greenColor = value;
                    redColor = value - delta;

                    hueStart = (1.0/3.0);
                }
                else if (hueRegion <= (4.0/6.0))
                {
                    // brightest: blue, green, red
                    blueColor = value;
                    redColor = value - delta;

                    hueStart = (2.0/3.0);
                }
                else if (hueRegion <= (5.0/6.0))
                {
                    // brightest: blue, red, green
                    blueColor = value;
                    greenColor = value - delta;

                    hueStart = (2.0/3.0);
                }
                else if (hueRegion <= 1.0)
                {
                    // brightest: red, blue, green
                    redColor = value;
                    greenColor = value - delta;

                    hueStart = 0;
                }

                double hueMod = ( hueFloatingPoint - hueStart );

                double hueModDelta = (delta/2 + (hueMod * value));

                if (hueRegion <= (1.0/6.0))
                {
                    greenColor = (hueModDelta + blueColor);
                }
                else if (hueRegion <= (2.0/6.0))
                {
                    redColor = (hueModDelta + blueColor);
                }
                else if (hueRegion <= (3.0/6.0))
                {
                    blueColor = (hueModDelta + redColor);
                }
                else if (hueRegion <= (4.0/6.0))
                {
                    greenColor = (hueModDelta + redColor);
                }
                else if (hueRegion <= (5.0/6.0))
                {
                    redColor = (hueModDelta + greenColor);
                }
                else if (hueRegion <= 1.0)
                {
                    blueColor = (hueModDelta + greenColor);
                }
            }

            // Normalize the colors.
            double hueRed = value - redColor;
            double hueBlue = value - blueColor;
            double hueGreen = value - greenColor;

            // Put it into our vector.
            uint8 brightnessColor = double2color(value);

            colorOut.red = brightnessColor - double2color(hueRed);
            colorOut.green = brightnessColor - double2color(hueGreen);
            colorOut.blue = brightnessColor - double2color(hueBlue);
        }

        inline void getHSVPropertiesOfColor(const texel_t& color, double& hueOut, vec4_t& vecOut) const
        {
            // Give the perfect transparency a special position.
            if (color.alpha == 0)
            {
                hueOut = 0;
                vecOut.x = 0;
                vecOut.y = 0;
                vecOut.z = 0;
                vecOut.w = -9001;
                return;
            }

            double brightness, saturation, hueColor;

            RGB2HSV(color, hueColor, saturation, brightness);

            // Construct the HUE color coordinate.
            double colorX = (cos(hueColor) * saturation);
            double colorY = (sin(hueColor) * saturation);

            // Create the blackness vector.
            double blacknessZ =
                (1.0 - brightness) * 1.7;

            // Create the alpha vector
            double alphaW =
                (1.0 - color2double(color.alpha)) * 2.4;

            // Construct the coordinate.
            vecOut.x = colorX;
            vecOut.y = colorY;
            vecOut.z = blacknessZ;
            vecOut.w = alphaW;

            hueOut = hueColor;
        }

        inline static double anglemorph(double angle, double morph)
        {
            bool needsRecursion = false;

            do
            {
                needsRecursion = false;

                if ( angle > morph )
                {
                    angle -= M_PI * 2;

                    needsRecursion = true;
                }
                else if ( angle < -morph )
                {
                    angle += M_PI * 2;

                    needsRecursion = true;
                }
            }
            while ( needsRecursion );

            return angle;
        }

        inline static double normalizeangledistance(double angle)
        {
            return anglemorph(angle, M_PI);
        }

        inline static double normalizeangle(double angle)
        {
            return anglemorph(angle, M_PI * 2);
        }

        inline static double anglediff(double first, double second)
        {
            return normalizeangledistance( first - second );
        }

        inline result_t getCriteria(const texel_t& first, const texel_t& second) const
        {
            double firstHUE, secondHUE;
            vec4_t firstVec, secondVec;

            // Calculate HSV color properties using an extended 4-dimensional model.
            getHSVPropertiesOfColor(first, firstHUE, firstVec);
            getHSVPropertiesOfColor(second, secondHUE, secondVec);
    
            // Calculate the distance between HUE angles.
            double angleDiff = fabs( anglediff(firstHUE, secondHUE) );

            // Construct the difference between the two vectors.
            vec4_t vecDiff;
            vecDiff.x = ( firstVec.x - secondVec.x );
            vecDiff.y = ( firstVec.y - secondVec.y );
            vecDiff.z = ( firstVec.z - secondVec.z );
            vecDiff.w = ( firstVec.w - secondVec.w );

            // Calculate the distance result.
            result_t result;
            result.angleDiff = angleDiff;
            result.distance = sqrt( vecDiff.x*vecDiff.x + vecDiff.y*vecDiff.y + vecDiff.z*vecDiff.z + vecDiff.w*vecDiff.w );

            double alphaColor = color2double(first.alpha);
            double alphaColor2 = color2double(second.alpha);

            result.alphaDist = fabs(alphaColor - alphaColor2);

            return result;
        }

        inline bool isSmaller(result_t first, result_t second) const
        {
            return
                first.getCriterion() < second.getCriterion();
        }
    };

    inline void characterize(uint8 red, uint8 green, uint8 blue, uint8 alpha)
    {
        // Define properties about this color and set up conversion hints.
#if 0
        brightnessCriteria bright(true);

        findpixelundercriteria(bright, this->brightest, 
#endif

        this->alphaZeroTexelCount++;
    }

    inline void after_characterize(void)
    {
        return;
    }

    template <typename numType>
    inline static numType weightedmiddle(numType first, numType second, numType firstWeight, numType secondWeight)
    {
        double weightCompositor = ( firstWeight + secondWeight );

        numType result = 0;

        if ( weightCompositor != 0 )
        {
            result = ( first * firstWeight + second * secondWeight ) / weightCompositor;
        }

        return result;
    }

    inline static uint8 colormiddle(uint8 first, uint8 second, double firstWeight, double secondWeight)
    {
        double firstColor = color2double(first);
        double secondColor = color2double(second);

        double weighted = weightedmiddle(firstColor, secondColor, firstWeight, secondWeight);

        return (uint8)( weighted * 255.0 );
    }

    colordiffCriteria crit;

    inline void feedcolor(uint8 red, uint8 green, uint8 blue, uint8 alpha)
    {
        // Try to find an already existing color that is same as the given one.
        bool texelExists = false;

        texel_t theTexel;

        theTexel.red = red;
        theTexel.green = green;
        theTexel.blue = blue;
        theTexel.alpha = alpha;

        for (uint32 n = 0; n < texelElimData.size(); n++)
        {
            texel_t& compTexel = texelElimData[n];

            bool isTheTexel = false;

            if ( !isTheTexel )
            {
                colordiffCriteria::result_t criterion = crit.getCriteria(compTexel, theTexel);

                isTheTexel =
                    ( criterion.angleDiff < colordiffCriteria::deg2rad(2.75) &&
                      criterion.distance < 0.15 &&
                      criterion.alphaDist < 0.075 );
            }

            if (isTheTexel)
            {
                // We already have this texel, so dont add it.
                texelExists = true;

                // Increase the usage count.
                compTexel.usageCount++;
            }
        }

        if ( !texelExists )
        {
            theTexel.usageCount++;

            // If the texel was not found, eliminate ourselves.
            texelElimData.push_back( theTexel );
        }
    }

    static inline bool sortByUsageCount(const texel_t& left, const texel_t& right)
    {
        return ( left.usageCount > right.usageCount );
    }

    struct similarPixels_t
    {
        const colordiffCriteria *crit;

        uint32 first_index;
        uint32 second_index;

        colordiffCriteria::result_t theResult;

        inline bool operator < ( const similarPixels_t& right ) const
        {
            return crit->isSmaller( theResult, right.theResult );
        }

        inline bool operator == ( const similarPixels_t& right ) const
        {
            return ( this->first_index == right.first_index && this->second_index == right.second_index ||
                     this->first_index == right.second_index && this->second_index == right.first_index );
        }
    };

    inline void constructpalette(uint32 maxentries)
    {
        // Construct similarity lists of texels while the amount of texels is too big.
        while ( texelElimData.size() > maxentries )
        {
            uint32 numTexels = texelElimData.size();

            typedef std::vector <similarPixels_t> simList_t;

            simList_t similarities;

            similarities.reserve( numTexels );

            for ( uint32 n = 0; n < numTexels; n++ )
            {
                uint32 smallest;
                colordiffCriteria::result_t smallest_result;

                bool hasFound = findmostsimilarpixel(crit, smallest_result, n, smallest);

                assert( hasFound == true );

                // Store it.
                similarPixels_t sim;
                sim.first_index = n;
                sim.second_index = smallest;
                sim.theResult = smallest_result;
                sim.crit = &crit;

                // Make sure this is not already in the list.
                if ( std::find( similarities.begin(), similarities.end(), sim ) == similarities.end() )
                {
                    similarities.push_back( sim );
                }
            }

            // Sort the list so that the most similar pixels come first.
            std::sort( similarities.begin(), similarities.end() );

            // Merge the colors to create a new color list.
            std::vector <texel_t> newColors;

            for ( simList_t::const_iterator iter = similarities.begin(); iter != similarities.end(); iter++ )
            {
                const similarPixels_t& sim = *iter;

                const colordiffCriteria::result_t& colorDiff = sim.theResult;

                uint32 first_diff = sim.first_index;
                uint32 second_diff = sim.second_index;

                bool isCloseEnough =
                    colorDiff.angleDiff < colordiffCriteria::deg2rad(1.5) &&
                    colorDiff.distance < 0.05 &&
                    colorDiff.alphaDist < 0.1;

                if (isCloseEnough)
                {
                    // Delete the texel that is used the least.
                    const texel_t& firstTexel = texelElimData.at( first_diff );
                    const texel_t& secondTexel = texelElimData.at( second_diff );

                    if ( firstTexel.usageCount < secondTexel.usageCount )
                    {
                        newColors.push_back( secondTexel );
                    }
                    else
                    {
                        newColors.push_back( firstTexel );
                    }
                }
                else
                {
                    // Generate a color between the color that is close to both colors.
                    // Then remove both colors, preferring the new one.
                    texel_t newTexel;
                    {
                        const texel_t& firstTexel = texelElimData.at( first_diff );
                        const texel_t& secondTexel = texelElimData.at( second_diff );

                        double hue1, saturation1, brightness1,
                               hue2, saturation2, brightness2;

                        double firstAlpha = color2double(firstTexel.alpha);
                        double secondAlpha = color2double(secondTexel.alpha);

                        double overallNormalWeight = ( firstAlpha + secondAlpha ) / 2;

                        double firstWeight = firstAlpha / overallNormalWeight;
                        double secondWeight = secondAlpha / overallNormalWeight;

                        double overallUsageCount = ( (double)firstTexel.usageCount + (double)secondTexel.usageCount ) / 2.0;

                        double firstAlphaWeight = (double)firstTexel.usageCount / overallUsageCount;
                        double secondAlphaWeight = ( 2 - firstAlphaWeight );
                        
                        colordiffCriteria::RGB2HSV(firstTexel, hue1, saturation1, brightness1);
                        colordiffCriteria::RGB2HSV(secondTexel, hue2, saturation2, brightness2);

                        // Calculate the middle color based on HSVA.
                        double hueMiddle =
                            colordiffCriteria::normalizeangle(
                                hue1 + colordiffCriteria::anglediff(hue2, hue1) / 2
                            );

                        double saturationMiddle = weightedmiddle(saturation1, saturation2, firstWeight, secondWeight);
                        double brightnessMiddle = weightedmiddle(saturation1, saturation2, firstWeight, secondWeight);

                        // Get the new RGB value.
                        colordiffCriteria::HSV2RGB(hueMiddle, saturationMiddle, brightnessMiddle, newTexel);

                        // Alpha is calculated conventionally.
                        newTexel.alpha = colormiddle(firstTexel.alpha, secondTexel.alpha, firstAlphaWeight, secondAlphaWeight);
                    }
                    newColors.push_back(newTexel);
                }
            }

            // Replace the colors.
            texelElimData = newColors;
        }
    }

    inline void* makepalette(eRasterFormat rasterFormat, eColorOrdering colorOrder)
    {
        uint32 palDepth = Bitmap::getRasterFormatDepth(rasterFormat);

        uint32 palItemCount = texelElimData.size();

        uint32 palDataSize = getRasterDataSize( palItemCount, palDepth );

        // Allocate a container for the palette.
        void *paletteData = new uint8[ palDataSize ];

        uint32 n = 0;

        for ( texelContainer_t::const_iterator iter = texelElimData.begin(); iter != texelElimData.end(); iter++ )
        {
            const texel_t& curTexel = *iter;

            puttexelcolor(paletteData, n++, rasterFormat, colorOrder, palDepth, curTexel.red, curTexel.green, curTexel.blue, curTexel.alpha);
        }

        return paletteData;
    }
    
    inline uint32 getclosestlink(uint8 red, uint8 green, uint8 blue, uint8 alpha)
    {
        // Find an index into the palette image that is closest to the given color.
        colordiffCriteria parser;

        texel_t theTexel;
        theTexel.red = red;
        theTexel.green = green;
        theTexel.blue = blue;
        theTexel.alpha = alpha;

        uint32 closestIndex;
        colordiffCriteria::result_t closest;
        bool hasClosest = false;

        uint32 index = 0;

        for (texelContainer_t::const_iterator iter = texelElimData.begin(); iter != texelElimData.end(); iter++)
        {
            const texel_t& curTexel = *iter;

            colordiffCriteria::result_t thisClose = parser.getCriteria(theTexel, curTexel);

            if (!hasClosest || parser.isSmaller(thisClose, closest))
            {
                closestIndex = index;
                closest = thisClose;

                if (!hasClosest)
                {
                    hasClosest = true;
                }
            }

            index++;
        }

        assert(hasClosest == true);

        return closestIndex;
    }
};

struct _fetch_texel_libquant_traverse
{
    NativeTextureD3D *platformTex;

    uint32 mipIndex;
};

static void _fetch_image_data_libquant(liq_color row_out[], int row_index, int width, void *user_info)
{
    _fetch_texel_libquant_traverse *info = (_fetch_texel_libquant_traverse*)user_info;

    NativeTextureD3D *platformTex = info->platformTex;

    // Fetch the color row.
    void *texelSource = platformTex->texels[ info->mipIndex ];

    eRasterFormat rasterFormat = platformTex->parent->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;

    uint32 itemDepth = platformTex->depth;

    eColorOrdering colorOrder = platformTex->colorOrdering;

    void *palColors = platformTex->palette;
    uint32 palColorCount = platformTex->paletteSize;

    for (int n = 0; n < width; n++)
    {
        uint32 colorIndex = PixelFormat::coord2index(n, row_index, width);

        // Fetch the color.
        uint8 r, g, b, a;

        browsetexelcolor(texelSource, paletteType, palColors, palColorCount, colorIndex, rasterFormat, colorOrder, itemDepth, r, g, b, a);

        // Store the color.
        liq_color& outColor = row_out[ n ];
        outColor.r = r;
        outColor.g = g;
        outColor.b = b;
        outColor.a = a;
    }
}

// Custom algorithm for palettizing image data.
void NativeTexture::convertToPalette(ePaletteType convPaletteFormat)
{
    if ( this->platform != PLATFORM_D3D8 && this->platform != PLATFORM_D3D9 )
        return;

    NativeTextureD3D *platformTex = (NativeTextureD3D*)this->platformData;

    if (convPaletteFormat != PALETTE_8BIT && convPaletteFormat != PALETTE_4BIT)
        return;

    // If the texture is DXT compressed, decompress it.
    if ( platformTex->dxtCompression )
    {
        platformTex->decompressDxt();
    }

    eRasterFormat rasterFormat = this->rasterFormat;
    ePaletteType paletteType = platformTex->paletteType;

    if (paletteType == convPaletteFormat)
        return;

    eColorOrdering colorOrder = platformTex->colorOrdering;

    void *srcPaletteData = platformTex->palette;
    uint32 srcPaletteCount = platformTex->paletteSize;

    // Get palette maximums.
    uint32 maxPaletteEntries = 0;
    uint8 newDepth = 0;

    if (convPaletteFormat == PALETTE_8BIT)
    {
        maxPaletteEntries = 256;
        newDepth = 8;
    }
    else if (convPaletteFormat == PALETTE_4BIT)
    {
        maxPaletteEntries = 16;
        newDepth = 4;
    }

    // Do the palettization.
    {
        uint32 mipmapCount = platformTex->mipmapCount;

        // Decide what palette system to use.
        ePaletteRuntimeType useRuntime = rw::rwInterface.GetPaletteRuntime();

        if (useRuntime == PALRUNTIME_NATIVE)
        {
            palettizer conv;

            // Linear eliminate unique texels.
            // Use only the first texture.
            if ( mipmapCount > 0 )
            {
                uint32 srcWidth = platformTex->width[0];
                uint32 srcHeight = platformTex->height[0];
                void *texelSource = platformTex->texels[0];
                uint32 srcDepth = platformTex->depth;

#if 0
                // First define properties to use for linear elimination.
                for (uint32 y = 0; y < srcHeight; y++)
                {
                    for (uint32 x = 0; x < srcWidth; x++)
                    {
                        uint32 colorIndex = PixelFormat::coord2index(x, y, srcWidth);

                        uint8 red, green, blue, alpha;
                        bool hasColor = browsetexelcolor(texelSource, paletteType, paletteData, maxpalette, colorIndex, rasterFormat, red, green, blue, alpha);

                        if ( hasColor )
                        {
                            conv.characterize(red, green, blue, alpha);
                        }
                    }
                }

                // Prepare the linear elimination.
                conv.after_characterize();
#endif

                // Linear eliminate.
                for (uint32 y = 0; y < srcHeight; y++)
                {
                    for (uint32 x = 0; x < srcWidth; x++)
                    {
                        uint32 colorIndex = PixelFormat::coord2index(x, y, srcWidth);

                        uint8 red, green, blue, alpha;
                        bool hasColor = browsetexelcolor(texelSource, paletteType, srcPaletteData, srcPaletteCount, colorIndex, rasterFormat, colorOrder, srcDepth, red, green, blue, alpha);

                        if ( hasColor )
                        {
                            conv.feedcolor(red, green, blue, alpha);
                        }
                    }
                }
            }

            // Construct a palette out of the remaining colors.
            conv.constructpalette(maxPaletteEntries);

            // Point each color from the original texture to the palette.
            uint32 itemDepth = platformTex->depth;

            for (uint32 n = 0; n < mipmapCount; n++)
            {
                // Create palette index memory for each mipmap.
                uint32 srcWidth = platformTex->width[n];
                uint32 srcHeight = platformTex->height[n];
                void *texelSource = platformTex->texels[n];

                uint32 itemCount = ( srcWidth * srcHeight );
                
                uint32 dataSize = 0;
                void *newTexelData = NULL;

                // Allocate appropriate memory.
                if (convPaletteFormat == PALETTE_4BIT)
                {
                    dataSize = PixelFormat::palette4bit::sizeitems( itemCount );
                }
                else if (convPaletteFormat == PALETTE_8BIT)
                {
                    dataSize = PixelFormat::palette8bit::sizeitems( itemCount );
                }

                newTexelData = new uint8[ dataSize ];

                for ( uint32 colorIndex = 0; colorIndex < itemCount; colorIndex++ )
                {
                    // Browse each texel of the original image and link it to a palette entry.
                    uint8 red, green, blue, alpha;
                    browsetexelcolor(texelSource, paletteType, srcPaletteData, srcPaletteCount, colorIndex, rasterFormat, colorOrder, itemDepth, red, green, blue, alpha);

                    uint32 paletteIndex = conv.getclosestlink(red, green, blue, alpha);

                    assert( paletteIndex < maxPaletteEntries );

                    // Store it in the palette data.
                    if (convPaletteFormat == PALETTE_4BIT)
                    {
                        ( (PixelFormat::palette4bit*)newTexelData )->setvalue(colorIndex, paletteIndex);
                    }
                    else if (convPaletteFormat == PALETTE_8BIT)
                    {
                        ( (PixelFormat::palette8bit*)newTexelData )->setvalue(colorIndex, paletteIndex);
                    }
                }

                // Replace texture data.
                if ( texelSource )
                {
                    delete [] texelSource;
                }

                platformTex->texels[n] = newTexelData;

                platformTex->dataSizes[n] = dataSize;
            }

            // Set the new depth of the texture.
            platformTex->depth = newDepth;

            // Delete the old palette data (if available).
            if (srcPaletteData != NULL)
            {
                delete [] srcPaletteData;
            }

            // Store the new palette texels.
            platformTex->palette = conv.makepalette(rasterFormat, colorOrder);
            platformTex->paletteSize = conv.texelElimData.size();
        }
        else if (useRuntime == PALRUNTIME_PNGQUANT)
        {
            liq_attr *quant_attr = liq_attr_create();

            assert( quant_attr != NULL );

            liq_set_max_colors(quant_attr, maxPaletteEntries);

            _fetch_texel_libquant_traverse main_traverse;

            main_traverse.platformTex = platformTex;
            main_traverse.mipIndex = 0;

            liq_image *quant_image = liq_image_create_custom(
                quant_attr, _fetch_image_data_libquant, &main_traverse,
                platformTex->width[0], platformTex->height[0],
                1.0
            );

            assert( quant_image != NULL );

            // Quant it!
            liq_result *quant_result = liq_quantize_image(quant_attr, quant_image);

            if (quant_result != NULL)
            {
                // Get the palette and remap all mipmaps.
                for (uint32 n = 0; n < mipmapCount; n++)
                {
                    uint32 mipWidth = platformTex->width[n];
                    uint32 mipHeight = platformTex->height[n];

                    uint32 palItemCount = ( mipWidth * mipHeight );

                    unsigned char *newPalItems = new unsigned char[ palItemCount ];

                    assert( newPalItems != NULL );

                    liq_image *srcImage = NULL;
                    bool newImage = false;

                    _fetch_texel_libquant_traverse thisTraverse;

                    if ( n == 0 )
                    {
                        srcImage = quant_image;
                    }
                    else
                    {
                        // Create a new image.
                        thisTraverse.platformTex = platformTex;
                        thisTraverse.mipIndex = n;

                        srcImage = liq_image_create_custom(
                            quant_attr, _fetch_image_data_libquant, &thisTraverse,
                            mipWidth, mipHeight,
                            1.0
                        );

                        newImage = true;
                    }

                    // Map it.
                    liq_write_remapped_image( quant_result, srcImage, newPalItems, palItemCount );

                    // Delete image (if newly allocated)
                    if (newImage)
                    {
                        liq_image_destroy( srcImage );
                    }

                    // Update the texels.
                    delete [] platformTex->texels[ n ];

                    bool hasUsedArray = false;

                    {
                        uint32 dataSize = 0;
                        void *newTexelArray = NULL;

                        if (convPaletteFormat == PALETTE_4BIT)
                        {
                            dataSize = PixelFormat::palette4bit::sizeitems( palItemCount );
                        }
                        else if (convPaletteFormat == PALETTE_8BIT)
                        {
                            dataSize = palItemCount;

                            newTexelArray = newPalItems;

                            hasUsedArray = true;
                        }

                        if ( !hasUsedArray )
                        {
                            newTexelArray = new uint8[ dataSize ];

                            // Copy over the items.
                            for ( uint32 n = 0; n < palItemCount; n++ )
                            {
                                uint32 resVal = newPalItems[ n ];

                                if (convPaletteFormat == PALETTE_4BIT)
                                {
                                    ( (PixelFormat::palette4bit*)newTexelArray )->setvalue(n, resVal);
                                }
                                else if (convPaletteFormat == PALETTE_8BIT)
                                {
                                    ( (PixelFormat::palette8bit*)newTexelArray)->setvalue(n, resVal);
                                }
                            }
                        }

                        // Set the texels.
                        platformTex->texels[ n ] = newTexelArray;
                        platformTex->dataSizes[ n ] = dataSize;
                    }

                    if ( !hasUsedArray )
                    {
                        delete [] newPalItems;
                    }
                }

                // Delete the old palette data.
                if (void *srcPalData = platformTex->palette)
                {
                    delete [] srcPalData;
                }

                // Update the texture depth.
                platformTex->depth = newDepth;

                // Update the texture palette data.
                {
                    const liq_palette *palData = liq_get_palette(quant_result);

                    uint32 newPalItemCount = palData->count;

                    uint32 palDepth = Bitmap::getRasterFormatDepth(rasterFormat);

                    uint32 palDataSize = getRasterDataSize( newPalItemCount, palDepth );

                    void *newPalArray = new uint8[ palDataSize ];

                    for ( unsigned int n = 0; n < newPalItemCount; n++ )
                    {
                        const liq_color& srcColor = palData->entries[ n ];

                        puttexelcolor(newPalArray, n, rasterFormat, colorOrder, palDepth, srcColor.r, srcColor.g, srcColor.b, srcColor.a);
                    }

                    // Update texture properties.
                    platformTex->palette = newPalArray;
                    platformTex->paletteSize = newPalItemCount;
                }
            }
            else
            {
                assert( 0 );
            }

            // Release resources.
            liq_image_destroy( quant_image );

            liq_attr_destroy( quant_attr );

            liq_result_destroy( quant_result );
        }
        else
        {
            assert( 0 );
        }
    }

    // Notify the raster about its new format.
    platformTex->paletteType = convPaletteFormat;

    // Since we changed the colors, update the alpha flag.
    platformTex->hasAlpha = platformTex->doesHaveAlpha();
}

};