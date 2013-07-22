local freetype = ft.create();

function createFontRenderer(gl)
    local renderer = {};
    local fonts = {};
    local fontHQwidth, fontHQheight = 40, 40;

    function renderer.createFont(file)
        local face = freetype.createFace(file);
        
        if not (face) then return false; end;
        
        local font = {};
        local heightCache = {};
        
        local function allocateHeight(charHeight)
            -- Make sure it is height measured in pixels.
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
            
            for n=1,255 do
                face.loadGlyph(face.getCharIndex(n));
                
                local bmp = face.getGlyphBitmap();
                
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
                    texInfo.advX = texInfo.advX / 64;
                    texInfo.advY = texInfo.advY / 64;
                    
                    -- Cache rendering offset of glyph
                    texInfo.offX, texInfo.offY = face.getGlyphMetricsHoriBearing();
                    
                    texInfo.offX = texInfo.offX / 64;
                    texInfo.offY = texInfo.offY / 64;
                    
                    charMap[n] = texInfo;
                    table.insert(allocatedChars, texInfo);
                end
            end
            
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

                        local nextX = x + item.advX;
                        local nextY = y + item.advY;
                        
                        if (nextX > _width) then
                            x = 0;
                            y = y + charHeight;
                            
                            nextX = item.advX;
                            nextY = y + item.advY;
                        end
                        
                        glyphMap.drawBitmap(src, x, y, 0, 0, "modulate");
                        
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
                    
                    glyphTex = gl.createTexture2D(glyphMap);
                end
                
                -- Reloop while setting parameters
                n = 1;
                
                local texWidth, texHeight = glyphTex.getSize();
                
                while (n <= numAllocated) do
                    local item = allocatedChars[n];
                    local glyphBmp = item.bitmap;
                    
                    -- Cache rendering offset of glyph
                    local offX = item.offX;
                    local offY = item.offY - charHeight;
                    
                    local width, height = glyphBmp.getWidth(), glyphBmp.getHeight();
                    
                    -- Get texCoords of the glyph on the map.
                    local x, y = item.glyphX, item.glyphY;
                    local scaledGlyphTexPosX = x / texWidth;
                    local scaledGlyphTexPosY = y / texHeight;
                    local scaledGlyphTexPosXend = scaledGlyphTexPosX + width / texWidth;
                    local scaledGlyphTexPosYend = scaledGlyphTexPosY + height / texHeight;
                    
                    item.batch = gl.makeBatch(
                        function(cmdType, x, y)
                            x = x + offX;
                            y = y - offY;
                        
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
                    
                    if (x > clientWidth) then
                        x = 0;
                        y = y + charHeight;
                    end
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
        
        return font;
    end
    
    return renderer;
end