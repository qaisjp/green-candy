local root = file.root;
local win32 = win32;
local gl = gl;
local ogl = gl;

local mainRenderWindow = false;
local glDriver = false;
local sessions = {};

local function tdelete(tab, val)
    for m,n in ipairs(tab) do
        if (n == val) then
            table.remove(tab, m);
            return true;
        end
    end
    
    return false;
end

local g_blendMode = "unknown";

function allocateDrawingSession()
    return class.construct(
        function(c)
            function getWindow()
                return mainRenderWindow;
            end
        
            function getDriver()
                return glDriver;
            end
            
            function getResolution()
                return mainRenderWindow.getClientSize();
            end
            
            function getRefCount()
                return #sessions;
            end
        
            function destroy()
                tdelete(sessions, c);
                
                if (#sessions == 0) then
                    mainRenderWindow.destroy();
                    mainRenderWindow = false;
                end
            end
            
            table.insert(sessions, c);
            
            if (#sessions == 1) then
                mainRenderWindow = win32.createDialog(640, 480);
                mainRenderWindow.setVisible(true);
                mainRenderWindow.setText("Main Window");
                
                mainRenderWindow.addEventHandler("onClose",
                    function()
                        quit();
                    end
                );
                
                glDriver = gl.create(mainRenderWindow);
                
                local clientWidth, clientHeight = mainRenderWindow.getClientSize();
                
                -- Set up the matrices.
                glDriver.matrixMode("GL_PROJECTION");
                glDriver.ortho(0, clientWidth, clientHeight, 0, 0, 1);
                glDriver.matrixMode("GL_MODELVIEW");
                glDriver.loadIdentity();
            end
        end
    );
end

-- OpenGL utilities

-- Creates a shader program.
function glCreateProgram(glDriver)
    return glDriver.createProgram();
end

-- Attaches a shader to the program
function glProgramAttachShader(program, shaderType, ...)
    local glDriver = program.getDriver();
    local shader = glDriver.createShader(shaderType);
    
    if not (shader) then return false; end;
    
    shader.shaderSource(...);
    
    if not (shader.compile()) then
        print("failed shader compilation: " .. shader.getInfoLog());
        
        shader.destroy();
        return false;
    end
    
    program.attachShader(shader);
    return shader;
end

function glUseProgram(glDriver, program)
    return glDriver.useProgram(program);
end

-- Links the program for usage (end of compilation stage).
function glProgramLink(program)
    return program.link();
end

function makeglwnd()
    local window = win32.createDialog(win32.getWindowRect(640, 480));
    window.setVisible(true);
    
    local glDriver = ogl.create(window);
    
    local quadBatch = glDriver.makeBatch(
        function(type, x, y, width, height, u, v)
            texCoord2d(0, 0);
            vertex2d(x, y);
            texCoord2d(0, v);
            vertex2d(x, y + height);
            texCoord2d(u, 0);
            vertex2d(x + width, y);
            texCoord2d(u, v);
            vertex2d(x + width, y + height);
            texCoord2d(0, v);
            vertex2d(x, y + height);
            texCoord2d(u, 0);
            vertex2d(x + width, y);
        end
    );
    
    local clientWidth, clientHeight = window.getClientSize();
    
    local fbo = glDriver.createFrameBuffer();
    local bmp = bitmap.new(clientWidth, clientHeight, "rgba");
    local tex = glDriver.createTexture2D(bmp, false, "clamp");
    local texWidth, texHeight = tex.getSize();
    
    print("width: " .. texWidth .. ", height: " .. texHeight);
    
    fbo.bindColorTexture(tex, "draw");
    
    glDriver.disable("GL_TEXTURE_2D");
    
    -- Set up the matrices.
    glDriver.matrixMode("GL_PROJECTION");
    glDriver.loadIdentity();
    --glDriver.ortho(texWidth, 0, 0, texHeight, 0, 1);
    glDriver.ortho(0, clientWidth, clientHeight, 0, 0, 1);
    glDriver.matrixMode("GL_MODELVIEW");
    glDriver.loadIdentity();
    
    fbo.runContext("draw",
        function()
            glDriver.clear("color");
            glDriver.clear("depth");
        
            glDriver.runBatch(quadBatch, "triangles", 0, 0, texWidth, texHeight, 1, 1);
        end
    );
    
    -- Set up the matrices.
    glDriver.matrixMode("GL_PROJECTION");
    glDriver.loadIdentity();
    glDriver.ortho(0, clientWidth, clientHeight, 0, 0, 1);
    glDriver.matrixMode("GL_MODELVIEW");
    glDriver.loadIdentity();
    
    glDriver.enable("GL_TEXTURE_2D");
    
    local program = glCreateProgram(glDriver);
    
    if (program) then
        local content = fileGetContent("gl/test.glsl");
        
        if not (glProgramAttachShader(program, "fragment", content)) or
            not (glProgramLink(program)) then
            
            program.destroy();
            program = false;
        end
        
        -- Set cool uniforms.
        program.setUniform("myColor", 1, 0, 1, 1);
    end
    
	-- Set up an awesome framebuffer
	glDriver.addEventHandler("onFrame",
		function()
			glDriver.clear("color");
			glDriver.clear("depth");
            
            local u, v = clientWidth / texWidth, clientHeight / texHeight;
            
            tex.bind();
            glDriver.runBatch(quadBatch, "triangles", 0, 0, clientWidth, clientHeight, u, v);
			
            -- Test fragment/pixel shader functionality
            if (program) then
                glUseProgram(glDriver, program);
                
                glDriver.runBatch(quadBatch, "triangles", 0, 0, clientWidth, clientHeight, 1, 1);
                
                glUseProgram(glDriver, nil);
            end
            
			glDriver.present();
		end
	);
end