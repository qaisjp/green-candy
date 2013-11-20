// Premultiplied alpha (short for PM) means that the color value is
// multiplied with the alpha before blending. We can do this in our
// fragment shader.
void main( void )
{
    gl_FragColor = vec4(gl_Color.rgb * gl_Color.a, gl_Color.a);
}