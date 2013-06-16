local win32 = win32;

function makewnd()
	local wnd = win32.createDialog(200, 200);
	wnd.setVisible(true);
	wnd.setPosition(150, 300);
	return wnd;
end