local drawSession = false;
local gl = false;
local fontRenderer = false;

-- Forward declarations.
local checkDrawSession;

-----------------------------------------------------------------------------------
----- OpenGL drawing interface implementation BEGIN ---------------------
-----------------------------------------------------------------------------------
local g_renderTarget = false;
local fontDir = file.createTranslator("C:/WINDOWS/FONTS/");
local privateFontDir = file.createTranslator(root.absPath("luaBench/fonts/"));

-- OpenGL declarations.
local quadBatch;
local colorQuadBatch;
local colorQuadTexcoordBatch;
local lineBatch;

local function drawQuad(x, y, width, height)
    gl.runBatch(quadBatch, "quads", x, y, width, height);
end

local function totexture(obj)
    local typename = type(obj);
    
    if (typename == "gltexture") then
        return obj;
    elseif (typename == "glframebuffer") then
        return obj.getColorAttachment(0);
    end
    
    return false;
end

local matDrawImage = matrixNew();

local function glRenderLock(method)
    if (g_renderTarget) then
        -- Render in the render target context
        g_renderTarget.runContext(method);
    else
        -- Render in the current OpenGL context
        gl.runContext(method);
    end
end

function dxDrawImage(x, y, width, height, 
        image, rotation, rotCenterOffX, rotCenterOffY, color, postGUI)
    
    if (rotation == nil) then	rotation = 0; end
    if (rotCenterOffX == nil) then rotCenterOffX = 0; end
    if (rotCenterOffY == nil) then rotCenterOffY = 0; end
    if (color == nil) then color = 0xFFFFFFFF; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    image = totexture(image);
    
    if not (image) then return false; end;
    
    gl.enable("GL_TEXTURE_2D");
    image.bind();
    
    local r, g, b, a = ogl.intToColor(color);
    
    gl.matrixMode("GL_MODELVIEW");
    gl.pushMatrix();
    
    local midOffX = width / 2 + rotCenterOffX;
    local midOffY = height / 2 + rotCenterOffY;
    
    matDrawImage.setEulerAngles(0, 0, rotation);
    matDrawImage.setPosition(x + midOffX, y + midOffY, 0);
    
    gl.loadMatrix(matDrawImage);
    
    glRenderLock(
        function()
            gl.runBatch(colorQuadBatch, "quads", -midOffX, -midOffY, width, height, r, g, b, a);
        end
    );

    gl.popMatrix();
    return true;
end

function dxDrawImageSection(x, y, width, height, u, v, usize, vsize,
        image, rotation, rotCenterOffX, rotCenterOffY, color, postGUI)
    
    if (rotation == nil) then	rotation = 0; end
    if (rotCenterOffX == nil) then rotCenterOffX = 0; end
    if (rotCenterOffY == nil) then rotCenterOffY = 0; end
    if (color == nil) then color = 0xFFFFFFFF; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    image = totexture(image);
    
    if not (image) then return false; end;
    
    gl.enable("GL_TEXTURE_2D");
    image.bind();
    
    local r, g, b, a = ogl.intToColor(color);
    
    gl.matrixMode("GL_MODELVIEW");
    gl.pushMatrix();
    
    local midOffX = width / 2 + rotCenterOffX;
    local midOffY = height / 2 + rotCenterOffY;
    
    matDrawImage.setEulerAngles(0, 0, rotation);
    matDrawImage.setPosition(x + midOffX, y + midOffY, 0);
    
    gl.multiplyMatrix(matDrawImage);
    
    local texWidth, texHeight = image.getSize();
    
    glRenderLock(
        function()
            gl.runBatch(colorQuadBatch, "quads", -midOffX, -midOffY, width, height, r, g, b, a, u / texWidth, v / texHeight, usize / texWidth, vsize / texHeight);
        end
    );

    gl.popMatrix();
    return true;
end

function dxDrawLine(startX, startY, endX, endY, color, width, posGUI)
    if (width == nil) then width = 1; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    gl.disable("GL_TEXTURE_2D");
    
    local r, g, b, a = ogl.intToColor(color);
    
    glRenderLock(
        function()
            gl.runBatch(lineBatch, "lines", startX, startY, endX, endY, r, g, b, a);
        end
    );
end

function dxDrawRectangle(x, y, width, height, color, postGUI)
    if (color == nil) then color = 0xFFFFFFFF; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    gl.disable("GL_TEXTURE_2D");
    
    glRenderLock(
        function()
            gl.runBatch(colorQuadBatch, "quads", x, y, width, height, ogl.intToColor(color));
        end
    );
end

-- Initialize all fonts required by the MTA runtime
local mtaFonts = {};

local function makeMTAFont(path, defaultSize, style)
    -- Make sure we have an active draw session.
    checkDrawSession();

    local file = fontDir.open(path, "rb");
    
    if not (file) then
        file = privateFontDir.open(path, "rb");
        
        if not (file) then
            return nil;
        end
    end
    
    local font = fontRenderer.createFont(file);
    file.destroy();
    
    if (font) then
        font.size = defaultSize;
        font.style = style;
    end
    
    return font;
end

function dxSetScissorRect(x, y, width, height)
    -- Make sure we have an active draw session.
    checkDrawSession();

    gl.scissor(x, y, width, height);
end

function dxEnableScissorRect(enable)
    -- Make sure we have an active draw session.
    checkDrawSession();

    if (enable) then
        gl.enable("GL_SCISSOR_TEST");
    else
        gl.disable("GL_SCISSOR_TEST");
    end
end

function dxDrawText(text, left, top, right, bottom, color, scale,
                                    alignX, alignY, clip, wordBreak, postGUI, colorCoded,
                                    subPixelPositioning)
    
    if (right == nil) then right = left; end
    if (bottom == nil) then bottom = top; end
    if (color == nil) then color = 0xFFFFFFFF; end
    if (scale == nil) then scale = 1; end
    if (font == nil) then font = "default"; end
    if (alignX == nil) then alignX = "left"; end
    if (alignY == nil) then alignY = "top"; end
    if (clip == nil) then clip = false; end
    if (wordBreak == nil) then wordBreak = false; end
    if (postGUI == nil) then postGUI = false; end
    if (colorCoded == nil) then colorCoded = false; end
    if (subPixelPositioning == nil) then subPixelPositioning = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local fontHandle = mtaFonts[font];
    
    if not (fontHandle) then
        fontHandle = mtaFont.default;
        
        if not (fontHandle) then return false; end;
    end
    
    if (colorCoded) then
        wordBreak = false;
    end
    
    local wasScissor;
    
    if (clip) then
        wasScissor = gl.isEnabled("GL_SCISSOR_TEST");
        
        if not (wasScissor) then
            gl.enable("GL_SCISSOR_TEST");
        end
        
        -- Specify the rendering cut out area.
        gl.scissor(left, top, right - left, bottom - top);
    end
    
    glRenderLock(
        function()
            -- TODO: properly support text rendering!
            fontHandle.drawString(text, left, top, fontHandle.size * scale);
        end
    );
    
    if (clip) then
        if not (wasScissor) then
            gl.disable("GL_SCISSOR_TEST");
        end
    end
    
    return true;
end

-- Change the maximum allocation size of fonts.
function dxSetFontHQRange(width, height)
    if (width < 1) then return false; end;
    if (height < 1) then return false; end;
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    fontHQwidth = width;
    fontHQheight = height;
    return true;
end

function dxGetFontHeight(scale, font)
    if (scale == nil) then scale = 1; end
    if (font == nil) then font = "default"; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local info = fontInfo[font];
    
    if not (info) then return false; end;
    
    return info.height * scale;
end

function dxGetTextWidth(text, scale, font)
    if (scale == nil) then scale = 1; end
    if (font == nil) then font = "default"; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local fontItem = mtaFonts[font];
    
    if not (fontItem) then return false; end;
    
    return fontItem.getStringWidth(text, fontItem.size * scale);
end

function dxCreateFont(filepath, size, bold)
    if (size == nil) then size = 9; end
    if (bold == nil) then bold = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    return makeMTAFont(filepath, size, bold);
end

function dxCreateTextureFromFile(filepath, textureFormat, mipmaps, textureEdge)
    if (textureFormat == nil) then textureFormat = "argb"; end
    if (mipmaps == nil) then mipmaps = true; end
    if (textureEdge == nil) then textureEdge = "wrap"; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local file = fileOpen(filepath);
    
    if not (file) then return false; end;
    
    -- Convert textureFormat to our internal format (if necessary)
    if (textureFormat == "argb") then
        textureFormat = "rgba";
    end
    
    -- Convert textureEdge to our internal format (if necessary)
    if (textureEdge == "wrap") then
        textureEdge = "repeat";
    end
    
    -- Attempt to load valid texture information
    local bmp = dds.load(file);
    
    if not (bmp) then
        file.seek(0, "set");
        bmp = jpeg.load(file);
        
        if not (bmp) then
            file.seek(0, "set");
            bmp = png.load(file);
            
            if not (bmp) then
                file.seek(0, "set");
                bmp = s2g.load(file);
                
                if not (bmp) then
                    file.destroy();
                    return false;
                end
            end
        end
    end
    
    file.destroy();
    
    -- TODO: support converting bitmap dataType
    
    return gl.createTexture2D(bmp, mipmaps, textureEdge);
end

function dxCreateTexture(...)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- Can take severial different types of parameters!
    return dxCreateTextureFromFile(...);
end

function dxCreateShader(filepath, priority, maxDistance, layered, elementTypes)
    if (priority == nil) then priority = 0; end
    if (maxDistance == nil) then maxDistance = 0; end
    if (layered == nil) then layered = false; end
    if (elementTypes == nil) then elementTypes = "world,vehicle,object,other"; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    -- TODO: support shaders.
    return false;
end

function dxCreateRenderTarget(width, height, withAlpha)
    if (withAlpha == nil) then withAlpha = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local fbo = gl.createFrameBuffer();
    local rbo = gl.createRenderBuffer(width, height, "GL_DEPTH24_STENCIL8");
    
    local bmp = bitmap.new(width, height, withAlpha and "rgba" or "rgb");
    local targetTex = gl.createTexture2D(bmp);
    bmp.destroy();
    
    fbo.bindColorTexture(targetTex);
    rbo.bindDepth(fbo);
    rbo.bindStencil(fbo);
    
    -- Mark resources to be destroyed
    rbo.destroy();
    targetTex.destroy();
    return fbo;
end

function dxCreateScreenSource(width, height)
    -- Make sure we have an active draw session.
    checkDrawSession();

    return dxCreateRenderTarget(width, height, false);
end

function dxGetMaterialSize(obj)
    -- Make sure we have an active draw session.
    checkDrawSession();

    local image = totexture(obj);
    
    if not (image) then return false; end;
    
    return image.getSize();
end

function dxSetShaderValue(theShader, parameterName, value)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: shader support
    return false;
end

function dxSetShaderTessellation(theShader, tessellationX, tessellationY)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: shader support
    return false;
end

function dxSetShaderTransform(theShader, rotationX, rotationY, rotationZ, rotationCenterOffsetX, rotationCenterOffsetY, rotationCenterOffsetZ,
                                                  bRotationCenterOffsetOriginIsScreen, perspectiveCenterOffsetX, perspectiveCenterOffsetY, bPerspectiveCenterOffsetOriginIsScreen)
    -- Make sure we have an active draw session.
    checkDrawSession();
                                                  
    -- TODO: shader support
    return false;
end

function dxSetRenderTarget(renderTarget, clear)
    -- Make sure we have an active draw session.
    checkDrawSession();

    if not (renderTarget) then
        g_renderTarget = false;
        return true;
    end
    
    g_renderTarget = renderTarget;
    
    if (clear) then
        glRenderLock(
            function()
                gl.clear("color");
                gl.clear("depth");
            end
        );
    end
    
    return true;
end

function dxUpdateScreenSource(screenSource, resampleNow)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: screen source support... maybe?
    return false;
end

function dxGetStatus()
    -- Make sure we have an active draw session.
    checkDrawSession();

    local info =
    {
        TestMode = false,
        VideoCardName = "OpenGL compatible hardware",
        VideoCardRAM = 0, -- todo
        VideoCardPSVersion = 1.0, -- todo
        VideoMemoryFreeForMTA = 0, -- todo
        VideoMemoryUsedByFonts = 0, -- todo
        VideoMemoryUsedByTextures = 0, -- todo
        VideoMemoryUsedByRenderTargets = 0, -- todo
        SettingWindowed = true,
        SettingFXQuality = 3,
        SettingDrawDistance = 100,
        SettingVolumetricShadows = false,
        SettingStreamingVideoMemoryForGTA = 64,
        AllowScreenUpload = false,
        DepthBufferFormat = "unknown",
        VideoCardMaxAnisotropy = 0,
        SettingAnisotropicFiltering = 0,
        SettingAntiAliasing = 0,
        SettingHeatHaze = false,
        SettingGrassEffect = false,
        Setting32BitColor = true
    };
    
    -- TODO: make real info!
    return info;
end

function dxSetTestMode(testMode)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: support this?
    return false;
end

function dxMakeBitmap(pixels)
    -- TODO: this is required for proper pixel manipulation.
    -- Should be called for every pixel string that should turn non constant.
    -- The return value is the bitmap used in the MTA pixel functions.
    -- This is required since the MTA pixel functions are a hack.
    return false;
end

function dxGetTexturePixels(...)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: support this?
    return false;
end

function dxSetTexturePixel(...)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: support this?
    return false;
end

function dxGetPixelsSize(pixels)
    -- TODO: support this?
    return 0, 0;
end

function dxGetPixelsFormat(pixels)
    -- TODO: support this?
    return "plain";
end

function dxConvertPixels(pixels, newFormat, quality)
    -- TODO: support this?
    return false;
end

function dxGetPixelColor(pixels, x, y)
    -- TODO: support this?
    return 0x00000000;
end

function dxSetPixelColor(pixels, x, y, r, g, b, a)
    -- TODO: support this?
    return false;
end

function dxSetBlendMode(mode)
    -- Make sure we have an active draw session.
    checkDrawSession();

    return drawSession.setBlendMode(mode);
end

function dxGetBlendMode()
    -- Make sure we have an active draw session.
    checkDrawSession();

    return drawSession.getBlendMode();
end

function dxGetDriver()
    return gl;
end

function dxGetResolution()
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    return drawSession.getResolution();
end

--------------------------------------------------------------------------
---- OpenGL draw function implementation END --------------------
--------------------------------------------------------------------------

-- Utility to make sure that globals are set on demand.
function checkDrawSession()
    if not (drawSession) then
        -- Set the driver globals.
        drawSession = allocateDrawingSession();
        gl = drawSession.getDriver();
        fontRenderer = createFontRenderer(gl);
        
        -- Set up the drawing environment
        quadBatch = gl.makeBatch(
            function(batchType, x, y, width, height)
                texCoord2d(0, 0);
                vertex2d(x, y);
                texCoord2d(1, 0);
                vertex2d(x + width, y);
                texCoord2d(1, 1);
                vertex2d(x + width, y + height);
                texCoord2d(0, 1);
                vertex2d(x, y + height);
            end
        );
        
        colorQuadBatch = gl.makeBatch(
            function(batchType, x, y, width, height, r, g, b, a)
                color4d(r, g, b, a);
            
                texCoord2d(0, 0);
                vertex2d(x, y);
                texCoord2d(1, 0);
                vertex2d(x + width, y);
                texCoord2d(1, 1);
                vertex2d(x + width, y + height);
                texCoord2d(0, 1);
                vertex2d(x, y + height);
            end
        );
        
        colorQuadTexcoordBatch = gl.makeBatch(
            function(batchType, x, y, width, height, r, g, b, a, u, v, umax, vmax)
                color4d(r, g, b, a);
            
                texCoord2d(u, v);
                vertex2d(x, y);
                texCoord2d(umax, v);
                vertex2d(x + width, y);
                texCoord2d(umax, vmax);
                vertex2d(x + width, y + height);
                texCoord2d(u, vmax);
                vertex2d(x, y + height);
            end
        );
        
        lineBatch = gl.makeBatch(
            function(batchType, x, y, tox, toy, r, g, b, a)
                vertex2d(x, y);
                vertex2d(tox, toy);
            end
        );
        
        -- Set up the default MTA fonts
        mtaFonts["default"] = makeMTAFont("tahoma.ttf", 15, "normal");
        mtaFonts["default-bold"] = makeMTAFont("tahomabd.ttf", 15, "bold");
        mtaFonts["clear"] = makeMTAFont("verdana.ttf", 15, "normal");
        mtaFonts["arial"] = makeMTAFont("arial.ttf", 15, "normal");
        mtaFonts["sans"] = makeMTAFont("micross.ttf", 15, "normal");
        mtaFonts["pricedown"] = makeMTAFont("pricedown.ttf", 30, "normal");
        mtaFonts["bankgothic"] = makeMTAFont("sabankgothic.ttf", 30, "normal");
        mtaFonts["diploma"] = makeMTAFont("unifont-5.1.20080907.ttf", 30, "normal");
        mtaFonts["beckett"] = makeMTAFont("unifont-5.1.20080907.ttf", 30, "normal");
                
        if (drawSession.getRefCount() == 1) then
            -- Set default OpenGL renderstates.
            gl.enable("GL_TEXTURE_2D");
            gl.enable("GL_BLEND");
            gl.disable("GL_DEPTH_TEST");
        
            -- Establish rendering defaults.
            dxSetBlendMode("blend");
        end
    end
end

-- MTA:Lua Interpreter only function
-- Returns the handle of the drawing window belonging to our session.
function getDrawingWindow()
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    return drawSession.getWindow();
end