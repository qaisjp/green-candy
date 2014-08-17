local ogl = gl; -- gl namespace, not the actual driver.
local drawSession = false;
local gl = false;   -- for convenience we call the driver "gl"
local fontRenderer = false;
local scriptRoot = scriptRoot;  -- fileTranslator where this script file is in.

-- Forward declarations.
local checkDrawSession;

-- Special utilities for drawing objects
local dx_elements = {};
local createElement = createElement;

local function dxCreateElement(elementType, internal)
    local elem = createElement(elementType, nil);
    
    function elem.resolve()
        return internal;
    end
    
    function elem.destroy()
        internal.destroy();
    
        dx_elements[elem] = nil;
    end
    
    dx_elements[elem] = true;
    return elem;
end

local function dxIsElement(val)
    return not (dx_elements[val] == nil);
end

local function dxResolve(obj)
    if (dxIsElement(obj)) then
        return obj.resolve();
    end
    
    return obj;
end

-----------------------------------------------------------------------------------
----- OpenGL drawing interface implementation BEGIN ---------------------
-----------------------------------------------------------------------------------
local g_renderTarget = false;
local g_blendMode = false;
local fontDir = file.createTranslator("C:/WINDOWS/FONTS/");
local root = file.root;
local privateFontDir = file.createTranslator(root.absPath("luaBench/fonts/"));

-- OpenGL declarations.
local quadBatch;
local colorQuadBatch;
local colorQuadTexcoordBatch;
local lineBatch;

-- GLSL blending shaders.
local blendModeShaders = {};
local currentShaderFramework = false;
local currentShaderProgram = false;

local function setRenderShader(name)
    if not (currentShaderFramework) then return false; end;

    local shader = currentShaderFramework[name];
    
    if not (shader) then return false; end;
    
    -- Activate the program.
    gl.useProgram(shader);
    
    -- Set up the new program.
    currentShaderProgram = shader;
    
    -- TODO: maybe set default values?
    return shader;
end

local function unsetRenderShader()
    local current = currentShaderProgram;
    
    if (current) then
        gl.useProgram(nil);
    
        currentShaderProgram = false;
    end
end

-- Render Element registry.
local renderTargetRegistry = {};

local function totexture(obj)
    if (dxIsElement(obj)) then
        obj = obj.resolve();
    end

    local typename = type(obj);
    
    if (typename == "gltexture") then
        return obj;
    elseif (typename == "glframebuffer") then
        return obj.getColorAttachment(0);
    elseif (renderTargetRegistry[obj]) then
        return obj;
    end
    
    return false;
end

-- Get resolution depending on context
local function getResolution()
    if (g_renderTarget) then
        return totexture(g_renderTarget).getSize();
    else
        -- Make sure we have an active draw session.
        checkDrawSession();

        return drawSession.getResolution();
    end
end

-- Transform coordinates depending on context
local function getContextCoordinates(x, y)
    local renderWidth, renderHeight = getResolution();

    if (g_renderTarget) then        
        x = renderWidth - x;
    end
    
    y = renderHeight - y;

    return x, y;
end

local function getContextRectangle(x, y, width, height)
    local renderWidth, renderHeight = getResolution();
    
    if (g_renderTarget) then
        x = renderWidth - x - width;
    end
    
    y = renderHeight - y - height;
    return x, y;
end

local g_curMatrixMode = false;

local function getMatrixMode()
    return g_curMatrixMode;
end

local function setMatrixMode(mode)
    g_curMatrixMode = mode;
    
    gl.matrixMode(mode);
end

local function drawQuad(x, y, width, height)
    gl.runBatch(quadBatch, "quads", x, y, width, height);
end

local matDrawImage = matrixNew();

local function glRenderLock(method)
    if (g_renderTarget) then
        -- Switch projection matrix to render-target mode.
        -- TODO: maybe support multiple texture attachments to the OpenGL FBO?
        local renderWidth, renderHeight = drawSession.getResolution();
        
        local prevMatrixMode = getMatrixMode();
        
        setMatrixMode("GL_PROJECTION");
        gl.pushMatrix();
        
        gl.loadIdentity();
        gl.ortho(0, renderWidth, 0, renderHeight, 0, 1);
        
        -- Render in the render target context
        dxResolve(g_renderTarget).runContext("draw", method);
        
        gl.popMatrix();
        
        -- Restore the previous matrix mode (if set)
        if (prevMatrixMode) then
            setMatrixMode(prevMatrixMode);
        end
    else
        -- Render in the current OpenGL context
        gl.runContext(method);
    end
end

local function multiplyQuadRotation(midPosX, midPosY, rotation)
    -- * Herp derp? How does this work? *
    --All vertices on the pipeline are transformed with the GL_MODELVIEW matrix
    -- before being sent to the GPU buffer. This means that they x, y, z coordinates
    -- are relative to the GL_MODELVIEW coordinate system.
    -- First the coordinates are rotated with z-euler-angle rotation+180.
    -- Then, their position is offset by x + midOffY, y + midOffY, so they reside
    -- at their correct position.
    -- You can see the vertex coordinates as base position of the vertexes from the 0,0
    -- space. Using GL_MODELVIEW, one can move this virtual space to another location.
    
    -- We first rotate, then offset the vertice!
    matDrawImage.setEulerAngles(0, 0, rotation);
    matDrawImage.setPosition(midPosX, midPosY, 0);
    
    gl.multiplyMatrix(matDrawImage);
end

function dxDrawImage(x, y, width, height, 
        image, rotation, rotCenterOffX, rotCenterOffY, color, postGUI)
    
    if (rotation == nil) then	rotation = 0; end
    if (rotCenterOffX == nil) then rotCenterOffX = 0; end
    if (rotCenterOffY == nil) then rotCenterOffY = 0; end
    if (color == nil) then color = 0xFFFFFFFF; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Normalize the coordinates.
    x = math.floor(x);
    y = math.floor(y);
    width = math.floor(width);
    height = math.floor(height);
    rotCenterOffX = math.floor(rotCenterOffX);
    rotCenterOffY = math.floor(rotCenterOffY);
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    image = totexture(image);
    
    if not (image) then return false; end;
    
    gl.enable("GL_TEXTURE_2D");
    image.bind();
    
    local r, g, b, a = ogl.intToColor(color);
    
    setMatrixMode("GL_MODELVIEW");
    gl.pushMatrix();
    
    local midOffX = width / 2 + rotCenterOffX;
    local midOffY = height / 2 + rotCenterOffY;
    
    multiplyQuadRotation(x + midOffX, y + midOffY, rotation);
    
    setRenderShader("image");
    
    glRenderLock(
        function()
            gl.runBatch(colorQuadBatch, "quads", -midOffX, -midOffY, width, height, r, g, b, a);
        end
    );
    
    unsetRenderShader();

    gl.popMatrix();
    return true;
end
registerDebugProxy("dxDrawImage",
    "number", "number", "number", "number", totexture, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }, { "opt", "boolean" }
);

function dxDrawImageSection(x, y, width, height, u, v, usize, vsize,
        image, rotation, rotCenterOffX, rotCenterOffY, color, postGUI)
    
    if (rotation == nil) then	rotation = 0; end
    if (rotCenterOffX == nil) then rotCenterOffX = 0; end
    if (rotCenterOffY == nil) then rotCenterOffY = 0; end
    if (color == nil) then color = 0xFFFFFFFF; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Normalize the coordinates.
    x = math.floor(x);
    y = math.floor(y);
    width = math.floor(width);
    height = math.floor(height);
    rotCenterOffX = math.floor(rotCenterOffX);
    rotCenterOffY = math.floor(rotCenterOffY);
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    image = totexture(image);
    
    if not (image) then return false; end;
    
    gl.enable("GL_TEXTURE_2D");
    image.bind();
    
    local r, g, b, a = ogl.intToColor(color);
    
    setMatrixMode("GL_MODELVIEW");
    gl.pushMatrix();
    
    local midOffX = width / 2 + rotCenterOffX;
    local midOffY = height / 2 + rotCenterOffY;
    
    multiplyQuadRotation(x + midOffX, y + midOffY, rotation);
    
    local texWidth, texHeight = image.getSize();
    
    setRenderShader("image_section");
    
    glRenderLock(
        function()
            gl.runBatch(colorQuadTexcoordBatch, "quads", -midOffX, -midOffY, width, height, r, g, b, a, u / texWidth, v / texHeight, usize / texWidth, vsize / texHeight);
        end
    );
    
    unsetRenderShader();

    gl.popMatrix();
    return true;
end
registerDebugProxy("dxDrawImageSection",
    "number", "number", "number", "number", "number", "number", "number", "number", totexture, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }, { "opt", "number" }, { "opt", "boolean" }
);

function dxDrawLine(startX, startY, endX, endY, color, width, postGUI)
    if (width == nil) then width = 1; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    gl.disable("GL_TEXTURE_2D");
    
    local r, g, b, a = ogl.intToColor(color);
    
    setRenderShader("line");
    
    glRenderLock(
        function()
            gl.runBatch(lineBatch, "lines", startX, startY, endX, endY, r, g, b, a);
        end
    );
    
    unsetRenderShader();
    return true;
end
registerDebugProxy("dxDrawLine",
    "number", "number", "number", "number", "number", { "opt", "number" }, { "opt", "boolean" }
);

function dxDrawRectangle(x, y, width, height, color, postGUI)
    if (color == nil) then color = 0xFFFFFFFF; end
    if (postGUI == nil) then postGUI = false; end
    
    -- Normalize the coordinates
    x = math.floor(x);
    y = math.floor(y);
    width = math.floor(width);
    height = math.floor(height);
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    gl.disable("GL_TEXTURE_2D");
    
    setRenderShader("rect");
    
    glRenderLock(
        function()
            gl.runBatch(colorQuadBatch, "quads", x, y, width, height, ogl.intToColor(color));
        end
    );
    
    unsetRenderShader();
    return true;
end
registerDebugProxy("dxDrawRectangle",
    "number", "number", "number", "number", { "opt", "number" }, { "opt", "boolean" }
);

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

    local left, top = getContextRectangle(x, y, width, height);
    
    gl.scissor(left, top, width, height);
end
registerDebugProxy("dxSetScissorRect",
    "number", "number", "number", "number"
);

function dxEnableScissorRect(enable)
    -- Make sure we have an active draw session.
    checkDrawSession();

    if (enable) then
        gl.enable("GL_SCISSOR_TEST");
    else
        gl.disable("GL_SCISSOR_TEST");
    end
end
registerDebugProxy("dxEnableScissorRect", "boolean");

local function tofont(val)
    return mtaFonts[val];
end

function dxDrawText(text, left, top, right, bottom, color, scale, font,
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
    
    -- Normalize the coordinates
    left = math.floor(left);
    top = math.floor(top);
    right = math.floor(right);
    bottom = math.floor(bottom);
    
    local fontHandle = tofont(font);
    
    if not (fontHandle) then
        fontHandle = mtaFont.default;
        
        if not (fontHandle) then return false; end;
    end
    
    if (colorCoded) then
        wordBreak = false;
    end
    
    local fontRenderHeight = fontHandle.size * scale;
    local pixelFontRenderHeight = fontHandle.getHeight( fontRenderHeight );
    
    local textLeft, textTop = false, false;
    
    -- Handle alignment
    if (alignX == "left") then
        textLeft = left;
    elseif (alignX == "center") then
        local textWidth = fontHandle.getStringWidth(text, fontRenderHeight);
        local width = right - left;
        textLeft = left + (width - textWidth) / 2;
    elseif (alignX == "right") then
        local textWidth = fontHandle.getStringWidth(text, fontRenderHeight);
        textLeft = right - textWidth;
    else
        return false;
    end
    
    if (alignY == "top") then
        textTop = top;
    elseif (alignY == "center") then
        local height = bottom - top;
        textTop = top + (height - pixelFontRenderHeight) / 2;
    elseif (alignY == "bottom") then
        textTop = bottom - pixelFontRenderHeight;
    else
        return false;
    end
    
    local shader = setRenderShader("text");
    
    if (shader) then
        shader.setUniform("drawColor", ogl.intToColor(color));
    end
    
    glRenderLock(
        function()
            local wasScissor;
        
            if (clip) then
                wasScissor = gl.isEnabled("GL_SCISSOR_TEST");
                
                if not (wasScissor) then
                    gl.enable("GL_SCISSOR_TEST");
                end
                
                -- Specify the rendering cut out area.
                local width, height = right - left, bottom - top;
                local scissorLeft, scissorTop = getContextRectangle(left, top, width, height);
                
                gl.scissor(scissorLeft, scissorTop, width, height);
            end
            
            -- TODO: properly support text rendering!
            fontHandle.drawString(text, textLeft, textTop, fontRenderHeight, color);
            
            if (clip) then
                if not (wasScissor) then
                    gl.disable("GL_SCISSOR_TEST");
                end
            end 
        end
    );
    
    unsetRenderShader();
    return true;
end
registerDebugProxy("dxDrawText",
    "string", "number", "number",
    { "opt", "number" },    -- right
    { "opt", "number" },    -- bottom
    { "opt", "number" },    -- color
    { "opt", "number" },    -- scale
    { "opt", tofont },    -- font
    { "opt", "string" },    -- alignX
    { "opt", "string" },    -- alignY
    { "opt", "boolean" },   -- clip
    { "opt", "boolean" },   -- wordBreak
    { "opt", "boolean" },   -- postGUI
    { "opt", "boolean" },   -- colorCoded
    { "opt", "boolean" }    -- subPixelPositioning
);

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
registerDebugProxy("dxSetFontHQRange", "number", "number");

function dxGetFontHeight(scale, font)
    if (scale == nil) then scale = 1; end
    if (font == nil) then font = "default"; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local info = tofont(font);
    
    if not (info) then return false; end;
    
    return info.getHeight( info.size ) * scale;
end
registerDebugProxy("dxGetFontHeight", { "opt", "number" }, { "opt", tofont });

function dxGetTextWidth(text, scale, font)
    if (scale == nil) then scale = 1; end
    if (font == nil) then font = "default"; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local fontItem = tofont(font);
    
    if not (fontItem) then return false; end;
    
    return fontItem.getStringWidth(text, fontItem.size * scale);
end
registerDebugProxy("dxGetTextWidth", "string", { "opt", "number" }, { "opt", tofont });

function dxCreateFont(filepath, size, bold)
    if (size == nil) then size = 9; end
    if (bold == nil) then bold = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    return dxCreateElement("dxfont", makeMTAFont(filepath, size, bold));
end
registerDebugProxy("dxCreateFont", "string", { "opt", "number" }, { "opt", "boolean" });

function dxGetFontHandle(handle)
    -- Make sure we have an active draw session.
    checkDrawSession();

    return tofont(handle);
end
registerDebugProxy("dxGetFontHandle", tofont);

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
    
    return dxCreateElement("texture", gl.createTexture2D(bmp, mipmaps, textureEdge));
end
registerDebugProxy("dxCreateTextureFromFile", "string", { "opt", "string" }, { "opt", "boolean" }, { "opt", "string" });

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
registerDebugProxy("dxCreateShader",
    "string",   -- filepath
    { "opt", "number" },    -- priority
    { "opt", "number" },    -- maxDistance
    { "opt", "boolean" },   -- layered
    { "opt", "string" } -- elementTypes
);

function dxCreateRenderTarget(width, height, withAlpha, withDepth)
    if (withAlpha == nil) then withAlpha = false; end
    if (withDepth == nil) then withDepth = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
    
    local fbo = gl.createFrameBuffer();
    local rbo = false;
    
    if (withDepth) then
        -- Only add the depth buffer if the user requested it.
        rbo = gl.createRenderBuffer(width, height, "GL_DEPTH24_STENCIL8");
        
        rbo.bindDepth(fbo, "draw");
        rbo.bindStencil(fbo, "draw");
        
        -- Flag it to destroy with the fbo.
        rbo.destroy();
    end
    
    local bmp = bitmap.new(width, height, withAlpha and "rgba" or "rgb");
    local targetTex = gl.createTexture2D(bmp, false, "clamp", "nearest");
    bmp.destroy();
    
    fbo.bindColorTexture(targetTex);
    
    -- Mark resources to be destroyed
    targetTex.destroy();
    return dxCreateElement("rendertarget", fbo);
end
registerDebugProxy("dxCreateRenderTarget",
    "number", "number", { "opt", "boolean" }, { "opt", "boolean" }
);

function dxCreateScreenSource(width, height)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: we should support this better.
    return dxCreateRenderTarget(width, height, false);
end
registerDebugProxy("dxCreateScreenSource", "number", "number");

function dxGetMaterialSize(obj)
    -- Make sure we have an active draw session.
    checkDrawSession();

    local image = totexture(obj);
    
    if not (image) then return false; end;
    
    return image.getSize();
end
registerDebugProxy("dxGetMaterialSize", totexture);

local function toshader(val)
    if (dxIsElement(val)) then
        val = val.resolve();
    end

    return false;
end

function dxSetShaderValue(theShader, parameterName, value)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: shader support
    return false;
end
registerDebugProxy("dxSetShaderValue", toshader, "string", "any");

function dxSetShaderTessellation(theShader, tessellationX, tessellationY)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: shader support
    return false;
end
registerDebugProxy("dxSetShaderTesselation", toshader, "number", "number");

function dxSetShaderTransform(theShader, rotationX, rotationY, rotationZ, rotationCenterOffsetX, rotationCenterOffsetY, rotationCenterOffsetZ,
                                                  bRotationCenterOffsetOriginIsScreen, perspectiveCenterOffsetX, perspectiveCenterOffsetY, bPerspectiveCenterOffsetOriginIsScreen)
    
    if (rotationCenterOffsetX == nil) then rotationCenterOffsetX = 0; end
    if (rotationCenterOffsetY == nil) then rotationCenterOffsetY = 0; end
    if (rotationCenterOffsetZ == nil) then rotationCenterOffsetZ = 0; end
    if (bRotationCenterOffsetOriginIsScreen == nil) then bRotationCenterOffsetOriginIsScreen = false; end
    if (perspectiveCenterOffsetX == nil) then perspectiveCenterOffsetX = 0; end
    if (perspectiveCenterOffsetY == nil) then perspectiveCenterOffsetY = 0; end
    if (bPerspectiveCenterOffsetOriginIsScreen == nil) then perspectiveCnterOffsetOriginIsScreen = false; end
    
    -- Make sure we have an active draw session.
    checkDrawSession();
                                                  
    -- TODO: shader support
    return false;
end
registerDebugProxy("dxSetShaderTransform",
    toshader,
    "number", "number", "number",
    { "opt", "number" }, { "opt", "number" }, { "opt", "number" },
    { "opt", "boolean" },
    { "opt", "number" }, { "opt", "number "},
    { "opt", "boolean" }
);

local function torendertarget(val)
    if (dxIsElement(val)) then
        val = val.resolve();
    end

    -- TODO
    return val;
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
registerDebugProxy("dxSetRenderTarget", { "opt", torendertarget }, { "opt", "boolean" });

local function toscreensource(val)
    if (dxIsElement(val)) then
        val = val.resolve();
    end

    -- TODO
    return val;
end

function dxUpdateScreenSource(screenSource, resampleNow)
    -- Make sure we have an active draw session.
    checkDrawSession();

    -- TODO: screen source support... maybe?
    return false;
end
registerDebugProxy("dxUpdateScreenSource", toscreensource, { "opt", "boolean" });

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
registerDebugProxy("dxSetTextMode", "string");

local function topixels(obj)
    if (dxIsElement(obj)) then
        obj = obj.resolve();
    end

    -- TODO
    return obj;
end

function dxMakeBitmap(pixels)
    -- TODO: this is required for proper pixel manipulation.
    -- Should be called for every pixel string that should turn non constant.
    -- The return value is the bitmap used in the MTA pixel functions.
    -- This is required since the MTA pixel functions are a hack.
    return false;
end
registerDebugProxy("dxMakeBitmap", "string");

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
registerDebugProxy("dxGetPixelsSize", topixels);

function dxGetPixelsFormat(pixels)
    -- TODO: support this?
    return "plain";
end
registerDebugProxy("dxGetPixelsFormal", topixels);

function dxConvertPixels(pixels, newFormat, quality)
    -- TODO: support this?
    return false;
end
registerDebugProxy("dxConvertPixels", topixels, "string", { "opt", "string" });

function dxGetPixelColor(pixels, x, y)
    -- TODO: support this?
    return 0x00000000;
end
registerDebugProxy("dxGetPixelColor", topixels, "number", "number");

function dxSetPixelColor(pixels, x, y, r, g, b, a)
    if (a == nil) then a = 255; end

    -- TODO: support this?
    return false;
end
registerDebugProxy("dxSetPixelColor",
    topixels, "number", "number", "number", "number", "number", { "opt", "number" }
);

function dxSetBlendMode(mode)
    -- Make sure we have an active draw session.
    checkDrawSession();

    if (mode == "blend") then
        gl.disable("GL_DEPTH_TEST");
        gl.disable("GL_CULL_FACE");
        gl.shadeModel("smooth");
        gl.enable("GL_BLEND");
        gl.blendFunc("GL_SRC_ALPHA", "GL_ONE_MINUS_SRC_ALPHA");
        gl.enable("GL_ALPHA_TEST");
        gl.alphaFunc("GL_GEQUAL", (1 / 255));
        gl.disable("GL_LIGHTING");
    elseif (mode == "modulate_add") then
        gl.disable("GL_DEPTH_TEST");
        gl.disable("GL_CULL_FACE");
        gl.shadeModel("smooth");
        gl.enable("GL_BLEND");
        gl.blendFunc("GL_ONE", "GL_ONE_MINUS_SRC_ALPHA");
        gl.enable("GL_ALPHA_TEST");
        gl.alphaFunc("GL_GEQUAL", (1 / 255));
        gl.disable("GL_LIGHTING");
    elseif (mode == "add") then
        gl.disable("GL_DEPTH_TEST");
        gl.disable("GL_CULL_FACE");
        gl.shadeModel("smooth");
        gl.enable("GL_BLEND");
        gl.blendFunc("GL_ONE", "GL_ONE_MINUS_SRC_ALPHA");
        gl.enable("GL_ALPHA_TEST");
        gl.alphaFunc("GL_GEQUAL", (1 / 255));
        gl.disable("GL_LIGHTING");
    else
        return false;
    end
    
    -- Set the current shader program
    currentShaderFramework = blendModeShaders[mode];
    
    g_blendMode = mode;
    return true;
end
registerDebugProxy("dxSetBlendMode", "string");

function dxGetBlendMode()
    -- Make sure we have an active draw session.
    checkDrawSession();

    return g_blendMode;
end

-- This function should return an OpenGL or Direct3D driver depending on
-- the active implementation.
function dxGetDriver()
    -- Make sure we have an active draw session.
    checkDrawSession();

    return gl;
end

function dxGetResolution()
    return getResolution();
end

-- Extended functions specially crafted for the Lua Interpreter


--------------------------------------------------------------------------
---- OpenGL draw function implementation END --------------------
--------------------------------------------------------------------------
local glCreateProgram = glCreateProgram;
local glProgramAttachShader = glProgramAttachShader;
local glUseProgram = glUseProgram;
local glProgramLink = glProgramLink;

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
                color4d(r, g, b, a);
            
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
        
        -- Compile shaders.        
        local function createBlendFramework(name)
            local fileRoot = file.createTranslator(scriptRoot.absPath(name .. "/"));
            
            if not (fileRoot) then return false; end;
            
            local function makeBlendModeFragShader(path)
                local file = fileRoot.open(path, "rb");
                
                if not (file) then
                    print("could not find shader file '" .. path .. "'");
                    return false;
                end
                
                local content = file.read(file.size());
                file.destroy();
                
                local program = glCreateProgram(gl);
                
                if (program) then
                    if not (glProgramAttachShader(program, "fragment", content)) or
                        not (glProgramLink(program)) then
                        
                        print("failed preparation of shader '" .. path .. "'");
                        
                        program.destroy();
                        program = false;
                    end
                    
                    if (program) then
                        -- Set constant values.
                        program.setUniform("texSample", 0); -- "texSample" is a sampler that identifies tex stage unit 0
                    end
                end
                
                return program;
            end
            
            return class.construct(
                function(c)
                    local shaders = {};
                
                    function __index(_, contextName)
                        local shader = shaders[contextName];
                        
                        if not (shader == nil) then
                            return shader;
                        end
                        
                        shader = makeBlendModeFragShader(contextName .. ".glsl");
                        
                        shaders[contextName] = shader;
                        return shader;
                    end
                end
            );
        end
        
        -- Compile shaders.
        blendModeShaders.blend = createBlendFramework("blend");
        blendModeShaders.modulate_add = createBlendFramework("modulate_add");
        blendModeShaders.add = createBlendFramework("add");
        
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