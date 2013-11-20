uniform sampler2D texSample;
uniform float4 drawColor;

void main( void )
{
    vec2 tex_coord = gl_TexCoord[0].st;

    gl_FragColor = gl_Color * drawColor * texture2D(texSample, tex_coord);
}