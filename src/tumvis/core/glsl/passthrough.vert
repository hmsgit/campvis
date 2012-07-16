void main() {
    // just pass everything through to fragment shader.
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = ftransform();
    gl_FrontColor = gl_Color;
}
