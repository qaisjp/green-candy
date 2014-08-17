local ogl = gl;
local freetype = ft.create();

function createFontRenderer(gl)
    local renderer = {};
    local fonts = {};
    local fontHQwidth, fontHQheight = 40, 40;
    local renderMode = "normal";
    
    function renderer.setRenderMode(mode)
        renderMode = mode;
        return true;
    end
    
    function renderer.getRenderMode()
        return renderMode;
    end

    function renderer.createFont(file)
        local face = freetype.createFace(file);
        
        if not (face) then return false; end;
        
        local font = {};
        local heightCache = {};
        
        local function allocateHeight(charHeight)
            -- Make sure it is height measured in pixels.
            -- Also only allow size up to the HQ limit.
            charHeight = math.min(fontHQheight, math.floor(charHeight));
            
            -- 4 should be minimal font size, seriously! :o
            if (charHeight < 4) then return false; end;
            
            local data = heightCache[charHeight];
            
            if (data) then
                return data;
            end
        
            local heightData = {};
            local charMap = {};
            
            heightData.charMap = charMap;
            
            face.setPixelSizes(0, charHeight);
            --face.setCharSize(charHeight * 64, charHeight * 64, 96, 96);
            
            local allocatedChars = {};
            local maxWidth = 0;
            
            local glyphOriginOffsetY = 0;
            local fontMax_yMin = 0;
            local fontMax_yMax = 0;
            
            for n=1,255 do
                face.loadGlyph(face.getCharIndex(n), 2);
                
                local bmp = face.getGlyphBitmap(renderMode);
                
                if (bmp) then
                    local texInfo = {
                        bitmap = bmp,
                        sizeMetrics = face.getSizeMetrics()
                    };
                
                    -- Get the maximum width for glyphMap creation space allocation
                    local width = bmp.getWidth();
                    
                    if (maxWidth < width) then
                        maxWidth = width;
                    end
                    
                    -- Cache glyph advance vector.
                    texInfo.advX, texInfo.advY = face.getGlyphAdvance();
                    
                    -- Convert it to pixel coords.
                    texInfo.advX = math.ceil(texInfo.advX / 64);
                    texInfo.advY = math.ceil(texInfo.advY / 64);
                    
                    -- Cache rendering offset of glyph
                    local horiBearingX, horiBearingY = face.getGlyphMetricsHoriBearing();
                    
                    local pixelHoriBearingX = math.floor(horiBearingX / 64);
                    local pixelHoriBearingY = math.floor(horiBearingY / 64);
                    
                    if ( glyphOriginOffsetY < pixelHoriBearingY ) then
                        glyphOriginOffsetY = pixelHoriBearingY;
                    end
                    
                    texInfo.offX = pixelHoriBearingX;
                    texInfo.offY = pixelHoriBearingY;
                    
                    local glyphWidth, glyphHeight = face.getGlyphMetricsSize();
                    
                    local yMin = math.ceil( ( glyphHeight - horiBearingY ) / 64 );
                    
                    if ( fontMax_yMin < yMin ) then
                        fontMax_yMin = yMin;
                    end
                    
                    local yMax = pixelHoriBearingY;
                    
                    if ( fontMax_yMax < yMax ) then
                        fontMax_yMax = yMax;
                    end
                    
                    texInfo.glyphWidth = math.ceil( glyphWidth / 64 );
                    texInfo.glyphHeight = math.ceil( glyphHeight / 64 );
                    
                    charMap[n] = texInfo;
                    table.insert(allocatedChars, texInfo);
                end
            end
            
            heightData.fontMax_yMin = fontMax_yMin;
            heightData.fontMax_yMax = fontMax_yMax;
            
            local numAllocated = #allocatedChars;
            local glyphMap = false;
            local glyphTex = false;
            
            if not (numAllocated == 0) then
                local n = 1;
                
                do
                    local x = 0;
                    local y = 0;
                    
                    local itemSquared = math.ceil(math.sqrt(numAllocated));
                    local _width = itemSquared * maxWidth;
                    local _height = itemSquared * charHeight;
                    
                    glyphMap = bitmap.new(_width, _height, "rgba");
                    
                    while (n <= numAllocated) do
                        local item = allocatedChars[n];
                        local src = item.bitmap;
                        local srcWidth = src.getWidth();

                        local nextX = x + srcWidth;
                        
                        if (nextX > _width) then
                            x = 0;
                            y = y + charHeight;
                            
                            nextX = srcWidth;
                        end
                        
                        local nextY = y;
                        
                        glyphMap.drawBitmap(src, x, y, 0, 0, "write");
                        
                        -- Store the glyph params.
                        -- We need to calculate rendering parameters once texture is allocated.
                        -- The texture might have been resized so bitmap coords do not equal
                        -- to texture coords.
                        item.glyphX = x;
                        item.glyphY = y;
                        
                        x = nextX;
                        y = nextY;
                    
                        n = n + 1;
                    end
                    
                    glyphTex = gl.createTexture2D(glyphMap, false, "clamp", "nearest");
                end
                
                -- Reloop while setting parameters
                n = 1;
                
                local texWidth, texHeight = glyphTex.getSize();
                
                while (n <= numAllocated) do
                    local item = allocatedChars[n];
                    local glyphBmp = item.bitmap;
                    
                    -- Cache rendering offset of glyph
                    local offX = item.offX;
                    local offY = glyphOriginOffsetY - item.offY;
                    
                    local bitmapWidth, bitmapHeight = glyphBmp.getWidth(), glyphBmp.getHeight();
                    
                    -- Get texCoords of the glyph on the map.
                    local x, y = item.glyphX, item.glyphY;
                    local scaledGlyphTexPosX = x / texWidth;
                    local scaledGlyphTexPosY = y / texHeight;
                    local scaledGlyphTexPosXend = scaledGlyphTexPosX + bitmapWidth / texWidth;
                    local scaledGlyphTexPosYend = scaledGlyphTexPosY + bitmapHeight / texHeight;
                    
                    -- Get the font width and height.
                    local width = item.glyphWidth;
                    local height = item.glyphHeight;
                    
                    if ( true ) then
                        width = bitmapWidth;
                        height = bitmapHeight;
                    end
                    
                    item.batch = gl.makeBatch(
                        function(cmdType, x, y)
                            x = x + offX;
                            y = y + offY;
                            
                            color4d(1, 1, 1, 1);
                        
                            if (cmdType == "triangles") then
                                texCoord2d(scaledGlyphTexPosX, scaledGlyphTexPosY);
                                vertex2d(x, y);
                                texCoord2d(scaledGlyphTexPosXend, scaledGlyphTexPosY);
                                vertex2d(x + width, y);
                                texCoord2d(scaledGlyphTexPosX, scaledGlyphTexPosYend);
                                vertex2d(x, y + height);
                                texCoord2d(scaledGlyphTexPosXend, scaledGlyphTexPosY);
                                vertex2d(x + width, y);
                                texCoord2d(scaledGlyphTexPosX, scaledGlyphTexPosYend);
                                vertex2d(x, y + height);
                                texCoord2d(scaledGlyphTexPosXend, scaledGlyphTexPosYend);
                                vertex2d(x + width, y + height);
                            elseif (cmdType == "quads") then
                                texCoord2d(scaledGlyphTexPosX, scaledGlyphTexPosY);
                                vertex2d(x, y);
                                texCoord2d(scaledGlyphTexPosXend, scaledGlyphTexPosY);
                                vertex2d(x + width, y);
                                texCoord2d(scaledGlyphTexPosXend, scaledGlyphTexPosYend);
                                vertex2d(x + width, y + height);
                                texCoord2d(scaledGlyphTexPosX, scaledGlyphTexPosYend);
                                vertex2d(x, y + height);
                            end
                        end
                    );
                
                    n = n + 1;
                end
            end
            
            -- Store the glyph texture
            heightData.glyphTex = glyphTex;
            
            heightCache[charHeight] = heightData;
            return heightData;
        end
        
        function font.drawString(str, x, y, height)
            local heightData = allocateHeight(height);
            
            if not (heightData) then return false; end;
        
            local n = 1;
            local max = #str;

            gl.enable("GL_TEXTURE_2D");
            heightData.glyphTex.bind();
            
            while (n <= max) do
                local char = string.byte(str, n);
                local tex = heightData.charMap[char];
                
                if (tex) then
                    gl.runBatch(tex.batch, "quads", x, y);
                
                    x = x + tex.advX;
                    y = y + tex.advY;
                end
                
                n = n + 1;
            end
        end
        
        function font.getStringWidth(str, height)
            local heightData = allocateHeight(height);
        
            if not (heightData) then return 0; end;
        
            local width = 0;
            local n = 1;
            local max = #str;
            
            while (n <= max) do
                local char = string.byte(str, n);
                local tex = heightData.charMap[char];
                
                if (tex) then
                    width = width + tex.advX;
                end
                
                n = n + 1;
            end
            
            return width;
        end
        
        function font.getHeight(height)
            local heightData = allocateHeight( height );
            
            if not (heightData) then return 0; end;
            
            return heightData.fontMax_yMax + heightData.fontMax_yMin;
        end
        
        -- Debug function
        function font.getFontTexture(height)
            local heightData = allocateHeight(height);
            
            if not (heightData) then return false; end;
            
            return heightData.glyphTex;
        end
        
        return font;
    end
    
    return renderer;
end