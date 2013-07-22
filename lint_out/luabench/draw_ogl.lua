local root = file.root;
local win32 = win32;
local gl = gl;
local ogl = gl;

local mainRenderWindow = false;
local glDriver = false;
local sessions = {};

local function tdelete(tab, val)
    for m,n in ipairs(sessions) do
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
            
            function setBlendMode(mode)
                if (mode == "blend") then
                    glDriver.disable("GL_DEPTH_TEST");
                    glDriver.disable("GL_CULL_FACE");
                    glDriver.shadeModel("smooth");
                    glDriver.enable("GL_BLEND");
                    glDriver.blendFunc("GL_SRC_ALPHA", "GL_ONE_MINUS_SRC_ALPHA");
                    glDriver.enable("GL_ALPHA_TEST");
                    glDriver.alphaFunc("GL_GEQUAL", (1 / 255));
                    glDriver.disable("GL_LIGHTING");
                
                    -- TODO: modulation states (in D3D: texture stage states)
                elseif (mode == "modulate_add") then
                    glDriver.disable("GL_DEPTH_TEST");
                    glDriver.disable("GL_CULL_FACE");
                    glDriver.shadeModel("smooth");
                    glDriver.enable("GL_BLEND");
                    glDriver.blendFunc("GL_ONE", "GL_ONE_MINUS_SRC_ALPHA");
                    glDriver.enable("GL_ALPHA_TEST");
                    glDriver.alphaFunc("GL_GEQUAL", 1);
                    glDriver.disable("GL_LIGHTING");
                    
                    -- TODO: modulation states
                elseif (mode == "add") then
                    glDriver.disable("GL_DEPTH_TEST");
                    glDriver.disable("GL_CULL_FACE");
                    glDriver.shadeModel("smooth");
                    glDriver.enable("GL_BLEND");
                    glDriver.blendFunc("GL_ONE", "GL_ONE_MINUS_SRC_ALPHA");
                    glDriver.enable("GL_ALPHA_TEST");
                    glDriver.alphaFunc("GL_GEQUAL", 1);
                    glDriver.disable("GL_LIGHTING");
                    
                    -- TODO: modulation states
                else
                    return false;
                end
                
                g_blendMode = mode;
                return true;
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
                glDriver.ortho(0, clientWidth, clientHeight, 0, 0, 1 );
                glDriver.matrixMode("GL_MODELVIEW");
                glDriver.loadIdentity();
            end
        end
    );
end

function makeglwnd()
	-- Set up an awesome framebuffer
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
			
			gl.present();
		end
	);
end