// Premultiplied alpha (short for PM) means that the color value is
// multiplied with the alpha before blending. We can do this in our
// fragment shader.
uniform sampler2D texSample;

void main( void )
{
    vec2 tex_coord = gl_TexCoord[0].st;
    
    vec4 out_color = texture2D(texSample, tex_coord) * gl_Color;

    gl_FragColor = vec4(out_color.rgb * out_color.a, out_color.a);
}