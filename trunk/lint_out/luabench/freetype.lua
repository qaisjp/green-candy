local freetype = ft.create();
local fontDir = file.createTranslator("C:/WINDOWS/fonts/");

function makeFace(path)
	local file = fontDir.open(path, "r");
	
	local face = freetype.createFace(file);
	
	file.destroy();
	
	return face;
end

function makeFaceBmp(path)
	local face = makeFace(path);
	
	if not (face) then return false; end;
	
	face.setPixelSizes(0, 16);
	face.loadGlyph(face.getCharIndexASCII("a"));
	
	local bitmap = face.getGlyphBitmap();
	
	if not (bitmap) then
		return false;
	end
	
	return face, bitmap;
end

local defs =
{
	"num_faces",
	"face_index",
	"face_flags",
	"style_flags",
	"num_glyphs",
	"family_name",
	"style_name",
	"num_fixed_sizes",
	"available_sizes",
	"num_charmaps",
	"units_per_EM",
	"ascender",
	"descender",
	"height",
	"max_advance_width",
	"max_advance_height",
	"underline_position",
	"underline_thickness",
	"size",
	"available_sizes",
	"charmaps"
};

function printFaceHash()
	local file = fileCreate("faceparams.txt");

	for m,n in ipairs(defs) do
		file.write(n .. ": " .. string.format("%.x", hashString(n)) .. "\n");
	end
	
	file.destroy();
end