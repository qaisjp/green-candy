uniform sampler2D texSample;

void main( void )
{
    vec2 tex_coord = gl_TexCoord[0].st;

    gl_FragColor = texture2D(texSample, tex_coord) * gl_Color;
}