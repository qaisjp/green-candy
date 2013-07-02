local root = file.root;
local win32 = win32;
local gl = gl;
local ogl = gl;

local fontDir = file.createTranslator("C:/WINDOWS/FONTS/");
local privateFontDir = file.createTranslator(root.absPath("luaBench/fonts/"));
local freetype = ft.create();

function makeglwnd()
	local dlg = win32.createDialog(640, 480);
	local gl = gl.create(dlg);
	
	dlg.setVisible(true);
	dlg.setText("OpenGL Test Device");
	
	local clientWidth, clientHeight = dlg.getClientSize();

	local quadBatch = gl.makeBatch(
		function(cmdType, x, y, width, height)
			texCoord2f(0, 0);
			vertex2f(x, y);
			texCoord2f(1.0, 0);
			vertex2f(x + width, y);
			texCoord2f(0, 1.0);
			vertex2f(x, y + height);
			texCoord2f(1.0, 0);
			vertex2f(x + width, y);
			texCoord2f(0, 1.0);
			vertex2f(x, y + height);
			texCoord2f(1.0, 1.0);
			vertex2f(x + width, y + height);
		end
	);
	
	-- Set up the matrices.
	gl.matrixMode("GL_PROJECTION");
	gl.ortho(0, clientWidth, clientHeight, 0, 0, 1 );
	gl.matrixMode("GL_MODELVIEW");
	gl.loadIdentity();
	
	local fonts = {};
	local fontHQwidth, fontHQheight = 40, 40;
	
	local function createFont(file)
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
	
	-----------------------------------------------------------------------------------
	----- OpenGL drawing interface implementation BEGIN ---------------------
	-----------------------------------------------------------------------------------
	local g_renderTarget = false;
	
    -- Set default OpenGL renderstates.
	gl.enable("GL_TEXTURE_2D");
	gl.enable("GL_BLEND");
	gl.disable("GL_DEPTH_TEST");
	
	local quadBatch = gl.makeBatch(
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
	
	local function drawQuad(x, y, width, height)
		gl.runBatch(quadBatch, "quads", x, y, width, height);
	end
	
	local colorQuadBatch = gl.makeBatch(
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
	
	local function dxDrawImage(x, y, width, height, 
			image, rotation, rotCenterOffX, rotCenterOffY, color, postGUI)
		
		if (rotation == nil) then	rotation = 0; end
		if (rotCenterOffX == nil) then rotCenterOffX = 0; end
		if (rotCenterOffY == nil) then rotCenterOffY = 0; end
		if (color == nil) then color = 0xFFFFFFFF; end
		if (postGUI == nil) then postGUI = false; end
		
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
	
	local colorQuadTexcoordBatch = gl.makeBatch(
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
	
	local function dxDrawImageSection(x, y, width, height, u, v, usize, vsize,
			image, rotation, rotCenterOffX, rotCenterOffY, color, postGUI)
		
		if (rotation == nil) then	rotation = 0; end
		if (rotCenterOffX == nil) then rotCenterOffX = 0; end
		if (rotCenterOffY == nil) then rotCenterOffY = 0; end
		if (color == nil) then color = 0xFFFFFFFF; end
		if (postGUI == nil) then postGUI = false; end
		
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
	
	local lineBatch = gl.makeBatch(
		function(batchType, x, y, tox, toy, r, g, b, a)
			vertex2d(x, y);
			vertex2d(tox, toy);
		end
	);
	
	local function dxDrawLine(startX, startY, endX, endY, color, width, posGUI)
		if (width == nil) then width = 1; end
		if (postGUI == nil) then postGUI = false; end
		
		gl.disable("GL_TEXTURE_2D");
		
		local r, g, b, a = ogl.intToColor(color);
		
		glRenderLock(
			function()
				gl.runBatch(lineBatch, "lines", startX, startY, endX, endY, r, g, b, a);
			end
		);
	end
	
	local function dxDrawRectangle(x, y, width, height, color, postGUI)
		if (color == nil) then color = 0xFFFFFFFF; end
		if (postGUI == nil) then postGUI = false; end
		
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
		local file = fontDir.open(path, "rb");
		
		if not (file) then
			file = privateFontDir.open(path, "rb");
			
			if not (file) then
				return nil;
			end
		end
		
		local font = createFont(file);
		file.destroy();
		
		if (font) then
			font.size = defaultSize;
			font.style = style;
		end
		
		return font;
	end
	
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
	
	local function dxSetScissorRect(x, y, width, height)
		gl.scissor(x, y, width, height);
	end
	
	local function dxEnableScissorRect(enable)
		if (enable) then
			gl.enable("GL_SCISSOR_TEST");
		else
			gl.disable("GL_SCISSOR_TEST");
		end
	end
	
	local function dxDrawText(text, left, top, right, bottom, color, scale,
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
	local function dxSetFontHQRange(width, height)
		if (width < 1) then return false; end;
		if (height < 1) then return false; end;
		
		fontHQwidth = width;
		fontHQheight = height;
		return true;
	end
	
	local function dxGetFontHeight(scale, font)
		if (scale == nil) then scale = 1; end
		if (font == nil) then font = "default"; end
		
		local info = fontInfo[font];
		
		if not (info) then return false; end;
		
		return info.height * scale;
	end
	
	local function dxGetTextWidth(text, scale, font)
		if (scale == nil) then scale = 1; end
		if (font == nil) then font = "default"; end
		
		local fontItem = mtaFonts[font];
		
		if not (fontItem) then return false; end;
		
		return fontItem.getStringWidth(text, fontItem.size * scale);
	end
	
	local function dxCreateFont(filepath, size, bold)
		if (size == nil) then size = 9; end
		if (bold == nil) then bold = false; end
		
		return makeMTAFont(filepath, size, bold);
	end
	
	local function dxCreateTextureFromFile(filepath, textureFormat, mipmaps, textureEdge)
		if (textureFormat == nil) then textureFormat = "argb"; end
		if (mipmaps == nil) then mipmaps = true; end
		if (textureEdge == nil) then textureEdge = "wrap"; end
		
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
	
	local function dxCreateTexture(...)
		-- Can take severial different types of parameters!
		return dxCreateTextureFromFile(...);
	end
	
	local function dxCreateShader(filepath, priority, maxDistance, layered, elementTypes)
		if (priority == nil) then priority = 0; end
		if (maxDistance == nil) then maxDistance = 0; end
		if (layered == nil) then layered = false; end
		if (elementTypes == nil) then elementTypes = "world,vehicle,object,other"; end
		
		-- TODO: support shaders.
		return false;
	end
	
	local function dxCreateRenderTarget(width, height, withAlpha)
		if (withAlpha == nil) then withAlpha = false; end
		
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
	
	local function dxCreateScreenSource(width, height)
		return dxCreateRenderTarget(width, height, false);
	end
	
	local function dxGetMaterialSize(obj)
		local image = totexture(obj);
		
		if not (image) then return false; end;
		
		return image.getSize();
	end
	
	local function dxSetShaderValue(theShader, parameterName, value)
		-- TODO: shader support
		return false;
	end
	
	local function dxSetShaderTessellation(theShader, tessellationX, tessellationY)
		-- TODO: shader support
		return false;
	end
	
	local function dxSetShaderTransform(theShader, rotationX, rotationY, rotationZ, rotationCenterOffsetX, rotationCenterOffsetY, rotationCenterOffsetZ,
													  bRotationCenterOffsetOriginIsScreen, perspectiveCenterOffsetX, perspectiveCenterOffsetY, bPerspectiveCenterOffsetOriginIsScreen)
		-- TODO: shader support
		return false;
	end
	
	local function dxSetRenderTarget(renderTarget, clear)
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
	
	local function dxUpdateScreenSource(screenSource, resampleNow)
		-- TODO: screen source support... maybe?
		return false;
	end
	
	local function dxGetStatus()
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
	
	local function dxSetTestMode(testMode)
		-- TODO: support this?
		return false;
	end
    
    local function dxMakeBitmap(pixels)
        -- TODO: this is required for proper pixel manipulation.
        -- Should be called for every pixel string that should turn non constant.
        -- The return value is the bitmap used in the MTA pixel functions.
        -- This is required since the MTA pixel functions are a hack.
        return false;
    end
	
	local function dxGetTexturePixels(...)
		-- TODO: support this?
		return false;
	end
	
	local function dxSetTexturePixel(...)
		-- TODO: support this?
		return false;
	end
	
	local function dxGetPixelsSize(pixels)
		-- TODO: support this?
		return 0, 0;
	end
	
	local function dxGetPixelsFormat(pixels)
		-- TODO: support this?
		return "plain";
	end
	
	local function dxConvertPixels(pixels, newFormat, quality)
		-- TODO: support this?
		return false;
	end
	
	local function dxGetPixelColor(pixels, x, y)
		-- TODO: support this?
		return 0x00000000;
	end
	
	local function dxSetPixelColor(pixels, x, y, r, g, b, a)
		-- TODO: support this?
		return false;
	end
    
    local g_blendMode = "unknown";
	
	local function dxSetBlendMode(mode)
        if (mode == "blend") then
            gl.disable("GL_DEPTH_TEST");
            gl.disable("GL_CULL_FACE");
            gl.shadeModel("smooth");
            gl.enable("GL_BLEND");
            gl.blendFunc("GL_SRC_ALPHA", "GL_ONE_MINUS_SRC_ALPHA");
            gl.enable("GL_ALPHA_TEST");
            gl.alphaFunc("GL_GEQUAL", (1 / 255));
            gl.disable("GL_LIGHTING");
        
            -- TODO: modulation states (in D3D: texture stage states)
        elseif (mode == "modulate_add") then
            gl.disable("GL_DEPTH_TEST");
            gl.disable("GL_CULL_FACE");
            gl.shadeModel("smooth");
            gl.enable("GL_BLEND");
            gl.blendFunc("GL_ONE", "GL_ONE_MINUS_SRC_ALPHA");
            gl.enable("GL_ALPHA_TEST");
            gl.alphaFunc("GL_GEQUAL", 1);
            gl.disable("GL_LIGHTING");
            
            -- TODO: modulation states
        elseif (mode == "add") then
            gl.disable("GL_DEPTH_TEST");
            gl.disable("GL_CULL_FACE");
            gl.shadeModel("smooth");
            gl.enable("GL_BLEND");
            gl.blendFunc("GL_ONE", "GL_ONE_MINUS_SRC_ALPHA");
            gl.enable("GL_ALPHA_TEST");
            gl.alphaFunc("GL_GEQUAL", 1);
            gl.disable("GL_LIGHTING");
            
            -- TODO: modulation states
        else
            return false;
        end
        
        g_blendMode = mode;
        return true;
    end
    
    local function dxGetBlendMode()
        return g_blendMode;
    end
    
    -- By default, we render using "blend" mode
    dxSetBlendMode("blend");
    
    --------------------------------------------------------------------------
    ---- OpenGL draw function implementation END --------------------
    --------------------------------------------------------------------------
	
	local fontFile = fontDir.open("tahoma.ttf", "rb");
	local myFont = false;
	
	if (fontFile) then
		myFont = createFont(fontFile);
		fontFile.destroy();
	end
	
	local testTex = dxCreateTexture("test.jpg");
	
	-- Set up an awesome framebuffer
	local fbo = gl.createFrameBuffer();
	
	local bmp = bitmap.new(clientWidth, clientHeight, "rgb");
	local fbo_tex = gl.createTexture2D(bmp, false);
	
	fbo.bindColorTexture(fbo_tex);
	
	gl.addEventHandler("onFrame",
		function()
			gl.clear("color");
			gl.clear("depth");
			
			if (myFont) then
				local height = 32;
				myFont.drawString("Hello World! I am a lazy fox.", 100, 100, height);
				myFont.drawString("I just jumped over the fence.", 100, 140, height);
				myFont.drawString(".......................................", 100, 180, height);
				
				myFont.drawString("Are you surprised?", 100, 240, height);
			end
			
			dxDrawText("trolololo face!", 100, 300);
			
			fbo.runContext("draw",
				function()
					gl.clear("color");
					gl.clear("depth");
					
					--dxDrawRectangle(-999999, -999999, 999999, 999999, 0xFFFFFFFF);
				
					dxDrawImage(0, 0, clientWidth, clientHeight, testTex);
				end
			);
			
			dxDrawLine(0, 0, clientWidth, clientHeight, 0xFFFFFF80);
			
			dxDrawImage(0, 0, clientWidth, clientHeight, fbo_tex);
			
			gl.present();
		end
	);
	
	dlg.addEventHandler("onClose",
		function()
			print("destroying OpenGL device");
		
			gl.destroy();
		end
	);
	return dlg, gl, fbo;
end