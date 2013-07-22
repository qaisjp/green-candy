local screenW, screenH = guiGetScreenSize();

addEventHandler("onClientRender", root,
    function()
        dxDrawRectangle(0, 0, screenW / 2, screenH / 2, tocolor(255, 255, 255, 255));
    end
);