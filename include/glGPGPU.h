#ifndef GLGPGPU_H
#define GLGPGPU_H


#ifdef ANDROID

/*

https://developer.android.com/ndk/guides/stable_apis

    OpenGL ES 1.0 - 3.2

    The standard OpenGL ES 1.x headers (<GLES/gl.h> and <GLES/glext.h>),
    2.0 headers (<GLES2/gl2.h> and <GLES2/gl2ext.h>),
    3.0 headers (<GLES3/gl3.h> and <GLES3/gl3ext.h>),
    3.1 headers (<GLES3/gl31.h> and <GLES3/gl3ext.h>), and
    3.2 headers (<GLES3/gl32.h> and <GLES3/gl3ext.h>) contain the declarations necessary for OpenGL ES.

*/

#include <GLES3/gl31.h>

#else

#ifdef _WIN32
#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif
#endif

#include <GL/glew.h>

#endif

#include <exception>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>

class GLRuntimeException: public std::exception
{

public:

    GLRuntimeException(std::string msg)
    : msg(msg)
    {}

private:

    virtual const char * what() const throw()
    {
        return msg.c_str();
    }

    std::string msg;

};

// print buffer status errors
GLuint glBufferStatus(const std::string msg)
{
    GLuint e = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch(e)
    {
        case GL_FRAMEBUFFER_UNDEFINED:
        throw( GLRuntimeException(msg+" GLERROR: GL_FRAMEBUFFER_UNDEFINED\n") );
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            throw( GLRuntimeException(msg+" GLERROR: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n") );
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            throw( GLRuntimeException(msg+" GLERROR: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n") );
            break;

        case GL_FRAMEBUFFER_UNSUPPORTED:
            throw( GLRuntimeException(msg+" GLERROR: GL_FRAMEBUFFER_UNSUPPORTED\n") );
            break;

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            throw( GLRuntimeException(msg+" GLERROR: GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n") );
            break;
    }
    return e;
}

// print gl error codes
GLuint glError(const std::string msg)
{
    GLuint e = glGetError();
    switch(e)
    {
        case GL_NO_ERROR:
        break;
        case GL_INVALID_ENUM:
            throw( GLRuntimeException(msg+" GL_INVALID_ENUM\n") );
            break;
        case GL_INVALID_VALUE:
            throw( GLRuntimeException(msg+" GLERROR: GL_INVALID_VALUE\n") );
            break;
        case GL_INVALID_OPERATION:
            throw( GLRuntimeException(msg+" GLERROR: GL_INVALID_OPERATION\n") );
            break;
        case GL_OUT_OF_MEMORY:
            throw( GLRuntimeException(msg+" GLERROR: GL_OUT_OF_MEMORY\n") );
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            throw( GLRuntimeException(msg+" GLERROR: GL_INVALID_FRAMEBUFFER_OPERATION\n") );
            break;
    }
    return e;
}

// compile a gl shader given a program and source code as const char *
void compileShader(GLuint & shaderProgram, const char * vert, const char * frag)
{
    GLuint vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader,1,&vert,NULL);
    glCompileShader(vertexShader);

    // check it worked!
    int  success;
    const unsigned logSize = 512*4;
    char infoLog[logSize];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(vertexShader, logSize, NULL, infoLog);
        throw( GLRuntimeException( std::string("GLSL (VERTEX) ERROR: \n") + infoLog + "\n"+vert+"\n") );
    }

    GLuint fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&frag,NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if(!success)
    {
        glGetShaderInfoLog(fragmentShader, logSize, NULL, infoLog);
        throw( GLRuntimeException( std::string("GLSL (FRAGMENT) ERROR: \n") + infoLog +"\n"+frag+"\n") );
    }

    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);

    // check it linked
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);
        throw( GLRuntimeException( std::string("GLSL (LINK) ERROR: \n") + infoLog + "\n"+vert+"\n"+frag+"\n") );
    }
    glGetProgramInfoLog(shaderProgram, logSize, NULL, infoLog);
}

void initTexture2DR32F(GLuint id, uint64_t n, uint64_t m)
{
    glBindTexture(GL_TEXTURE_2D,id);
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_NEAREST
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_NEAREST
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R32F,
        n,
        m,
        0,
        GL_RED,
        GL_FLOAT,
        NULL
    );
}

void transferToTexture2DR32F(GLuint id, std::vector<float> data, uint64_t n, uint64_t m)
{
    glBindTexture(GL_TEXTURE_2D,id);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R32F,
        n,
        m,
        0,
        GL_RED,
        GL_FLOAT,
        data.data()
    );
}

struct AbstractUniform
{
    AbstractUniform(std::string n)
    : name(n)
    {}

    virtual ~AbstractUniform() = default;

    std::string name;
};

template <class T>
struct Uniform : public AbstractUniform
{
    Uniform(std::string n, T v)
    : AbstractUniform(n), value(v)
    {}

    T value;
};

template <class T>
const Uniform NULL_UNIFORM("", static_cast<T>(NAN));

struct Sampler2D {

    Sampler2D(int t)
    : texture(t)
    {}

    Sampler2D(unsigned t)
    : texture(t)
    {}

    Sampler2D(float t)
    : texture(int(t))
    {}

    int texture;
};

template <class T>
const std::regex UNIFORM_DATA_REGEX;

template <> inline
const std::regex UNIFORM_DATA_REGEX<int> = std::regex("uniform int (\\S+);");
template <> inline
const std::regex UNIFORM_DATA_REGEX<float> = std::regex("uniform float (\\S+);");
template <> inline
const std::regex UNIFORM_DATA_REGEX<glm::vec2> = std::regex("uniform vec2 (\\S+);");
template <> inline
const std::regex UNIFORM_DATA_REGEX<glm::vec4> = std::regex("uniform vec4 (\\S+);");
template <> inline
const std::regex UNIFORM_DATA_REGEX<glm::mat4> = std::regex("uniform mat4 (\\S+);");
template <> inline
const std::regex UNIFORM_DATA_REGEX<Sampler2D> = std::regex("uniform(\\slowp\\s|\\shighp\\s|\\smediump\\s|\\s)sampler2D (\\S+);");


struct Shader
{
    Shader(const char * v, const char * f)
    : vertex(v), fragment(f)
    {
        parseUniforms();
    }

    Shader()
    : vertex(""),fragment("")
    {}

    Shader(std::string path, std::string name)
    {
        std::ifstream fileVs(path+name+".vs");
        std::ifstream fileFs(path+name+".fs");
        if (fileVs.is_open() && fileFs.is_open())
        {
            vertex = parseShaderSource(fileVs);
            fragment = parseShaderSource(fileFs);
        }
        else
        {
            throw std::runtime_error(" attempting to locate source files .vs and .fs at "+path+name);
        }

        parseUniforms();
    }

    virtual ~Shader() = default;

    bool operator==(const Shader & s)
    {
        return this->vertex == s.vertex && this->fragment == s.fragment;
    }

    template <class T>
    void setUniform(std::string name, T value)
    {
        if (uniforms.find(name) == uniforms.end())
        {
            throw std::runtime_error("could not find uniform: " + name);
        }

        AbstractUniform * uniform = uniforms[name].get();

        if (dynamic_cast<Uniform<T>*>(uniform) == nullptr)
        {
            return;
        }

        Uniform<T> * u = dynamic_cast<Uniform<T>*>(uniform);
        if (u != nullptr){ setValue(u, value); return; }

        return;
    }

    const std::string & getVertex() const { return vertex; }
    const std::string & getFragment() const { return fragment; }

    template <class T>
    Uniform<T> getUniform(std::string name)
    {
        if (uniforms.find(name) == uniforms.end())
        {
            return NULL_UNIFORM<T>;
        }

        AbstractUniform * uniform = uniforms[name].get();

        if (dynamic_cast<Uniform<T>*>(uniform) == nullptr)
        {
            return NULL_UNIFORM<T>;
        }

        Uniform<T> * u = dynamic_cast<Uniform<T>*>(uniform);
        if (u != nullptr){ return *u; }

        return NULL_UNIFORM<T>;
    }

    std::vector<std::string> getUniformNames()
    {
        std::vector<std::string> v;
        for (auto it = uniforms.cbegin(); it != uniforms.cend(); it++)
        {
            v.push_back(it->first);
        }
        return v;
    }

    virtual void use() = 0;

protected:

    std::string vertex;
    std::string fragment;

    std::unordered_map<std::string, std::shared_ptr<AbstractUniform>> uniforms;

    std::string parseShaderSource(std::ifstream & file)
    {
        std::string src = "";
        std::string line;
        while (std::getline(file,line))
        {
            src += line + "\n";
        }
        return src;
    }

    virtual void compile() = 0;

    bool parseUniforms()
    {
        std::regex UNIFORM_REGEX("uniform");
        bool fragmentNoUniforms = false;
        auto start = std::sregex_iterator(fragment.begin(), fragment.end(), UNIFORM_REGEX);
        auto end = std::sregex_iterator();
        if (std::distance(start, end) == 0)
        {
            fragmentNoUniforms = true;
        }

        start = std::sregex_iterator(vertex.begin(), vertex.end(), UNIFORM_REGEX);
        end = std::sregex_iterator();
        if (std::distance(start, end) == 0)
        {
            if (fragmentNoUniforms)
            {
                return true;
            }
        }

        std::vector<std::string> toCheck {vertex, fragment};

        for (unsigned i = 0; i < toCheck.size(); i++)
        {
            std::string code = toCheck[i];

            detectUniformsAndCreate<int>(code);
            detectUniformsAndCreate<float>(code);
            detectUniformsAndCreate<glm::vec2>(code);
            detectUniformsAndCreate<glm::vec4>(code);
            detectUniformsAndCreate<glm::mat4>(code);
            detectUniformsAndCreate<Sampler2D>(code);
        }

        return true;

    }

    template <class T>
    void detectUniformsAndCreate(std::string code)
    {
        const std::regex r = UNIFORM_DATA_REGEX<T>;

        auto start = std::sregex_iterator(code.begin(), code.end(), r);
        auto end = std::sregex_iterator();
        for (std::sregex_iterator it = start; it != end; it++)
        {
            std::string match = (*it).str();
            if (match.find("lowp") != std::string::npos)
            {
                // uniform lowp TYPE NAME;
                match.erase(0, match.find("lowp")+4);
                // " TYPE NAME;"
            }
            if (match.find("highp") != std::string::npos)
            {
                // uniform highp TYPE NAME;
                match.erase(0, match.find("highp")+5);
                // " TYPE NAME;"
            }
            // found "uniform TYPE NAME;" or " TYPE NAME;"
            match.erase(0, match.find(" ")+1);
            match.erase(0, match.find(" ")+1);
            std::string name = match.substr(0, match.find(";"));
            std::shared_ptr<Uniform<T>> u = std::make_shared<Uniform<T>>(Uniform<T>(name,T(0)));
            uniforms[name] = u;
        }

    }

    // cannot have spec in class scope https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
    //  also cannot use partial spec workaround because non-class, non-variable partial
    //  specialization is not allowed
    //  https://stackoverflow.com/questions/8061456/c-function-template-partial-specialization
    // so the template dispatches to these guys

    virtual void setValue(Uniform<int> * u, int value)
    {
        u->value = value;

    }

    virtual void setValue(Uniform<Sampler2D> * u, Sampler2D value)
    {
        u->value = value;
    }

    virtual void setValue(Uniform<float> * u, float value)
    {
        u->value = value;
    }

    virtual void setValue(Uniform<glm::vec2> * u, glm::vec2 value)
    {
        u->value = value;
    }

    virtual void setValue(Uniform<glm::vec4> * u, glm::vec4 value)
    {
        u->value = value;
    }

    virtual void setValue(Uniform<glm::mat4> * u, glm::mat4 value)
    {
        u->value = value;
    }
};

struct glShader : public Shader
{

    glShader(const char * v, const char * f)
    : Shader(v, f), program(0), compiled(false), used(false)
    {}

    glShader()
    : Shader(), program(0), compiled(false), used(false)
    {}

    glShader(std::string path, std::string name)
    : Shader(path, name), program(0), compiled(false), used(false)
    {}

    ~glShader(){if(isProgram()){release();}}

    void create()
    {
        if (!isProgram())
        {
            program = glCreateProgram();
            compiled = false;
        }
    }

    void release()
    {
        if (isProgram())
        {
            glDeleteProgram(program);
            compiled = false;
        }
    }

    void compile()
    {
        if (!isProgram())
        {
            create();
        }

        parseUniforms();

        compileShader(program,vertex.c_str(),fragment.c_str());
        compiled = true;

        for (auto uniform = uniforms.cbegin(); uniform != uniforms.cend(); uniform++)
        {
            AbstractUniform * u = (*uniform).second.get();

            Uniform<int> * ui = dynamic_cast<Uniform<int>*>(u);
            if (ui != nullptr){ setValue(ui, 0); continue; }

            Uniform<float> * uf = dynamic_cast<Uniform<float>*>(u);
            if (uf != nullptr){ setValue(uf, 0.0f); continue; }

            Uniform<glm::vec2> * uv = dynamic_cast<Uniform<glm::vec2>*>(u);
            if (uv != nullptr){ setValue(uv, glm::vec2(0.0f)); continue; }

            Uniform<glm::mat4> * um = dynamic_cast<Uniform<glm::mat4>*>(u);
            if (um != nullptr){ setValue(um, glm::mat4(0.0f)); continue; }
        }
    }

    void use()
    {
        if (!compiled){compile();}
        glUseProgram(program);
    }

    bool isCompiled(){return compiled;}
    bool isProgram(){return glIsProgram(program);}

private:

    GLuint program;
    bool compiled;
    bool used;

    inline const GLuint location(const char * name) const
    {
        return glGetUniformLocation(program, name);
    }

    // cannot have spec in class scope https://gcc.gnu.org/bugzilla/show_bug.cgi?id=85282
    //  also cannot use partial spec workaround because non-class, non-variable partial
    //  specialization is not allowed
    //  https://stackoverflow.com/questions/8061456/c-function-template-partial-specialization
    // so the template dispatches to these guys

    void setValue(Uniform<int> * u, int value)
    {
        u->value = value;
        if (isCompiled())
        {
            use();
            glUniform1i(location(u->name.c_str()), u->value);
        }

    }

    void setValue(Uniform<Sampler2D> * u, Sampler2D value)
    {
        u->value = value;
        if (isCompiled())
        {
            use();
            glUniform1i(location(u->name.c_str()), u->value.texture);
        }

    }

    void setValue(Uniform<float> * u, float value)
    {
        u->value = value;
        if (isCompiled())
        {
            use();
            glUniform1f(location(u->name.c_str()), u->value);
        }

    }

    void setValue(Uniform<glm::vec2> * u, glm::vec2 value)
    {
        u->value = value;
        if (isCompiled())
        {
            use();
            glUniform2f(location(u->name.c_str()), u->value.x, u->value.y);
        }

    }

    void setValue(Uniform<glm::vec4> * u, glm::vec4 value)
    {
        u->value = value;
        if (isCompiled())
        {
            use();
            glUniform4f(location(u->name.c_str()), u->value.x, u->value.y, u->value.z, u->value.w);
        }

    }

    void setValue(Uniform<glm::mat4> * u, glm::mat4 value)
    {
        u->value = value;
        if (isCompiled())
        {
            use();
            glUniformMatrix4fv(location(u->name.c_str()), 1, false, glm::value_ptr(u->value));
        }
    }

};

class glCompute
{

public:

    glShader shader;

    const char * vertexShader =
        "#version " GLSL_VERSION "\n"
        "precision highp float;\n"
        "precision highp int;\n"
        "layout(location = 0) in vec4 a_position;\n"
        "out vec2 o_texCoords;\n"
        "void main(){\n"
        "   gl_Position = vec4(a_position.xy,0.0,1.0);\n"
        "   o_texCoords = a_position.zw;\n"
        "}";

    glCompute
    (
        std::map<std::string, std::pair<uint64_t, uint64_t>> attributeSize,
        std::pair<uint64_t, uint64_t> outputSize,
        const char * fragmentShader
    )
    : outputSize(outputSize)
    {
        shader = glShader(vertexShader, fragmentShader);
        shader.compile();
        uint64_t n = attributeSize.size();
        textures.resize(n+1);
        glGenTextures(n+1, textures.data());
        uint64_t t = 0;
        for (auto & attr : attributeSize)
        {
            attributes[attr.first] = Attribute
            (
                std::vector<float>(attr.second.first*attr.second.second, 0.0),
                textures[t],
                attr.second.first,
                attr.second.second
            );
            initTexture2DR32F(textures[t], attr.second.first, attr.second.second);
            t++;
        }
        initTexture2DR32F(textures.back(), outputSize.first, outputSize.second);
        output = std::vector<float>(outputSize.first*outputSize.second, 0.0);
        transferToTexture2DR32F(textures.back(), output, outputSize.first, outputSize.second);
        glGenFramebuffers(1, &frameBuffer);
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData
        (
            GL_ARRAY_BUFFER,
            sizeof(float)*6*4,
            &quad[0],
            GL_STATIC_DRAW
        );
        glEnableVertexAttribArray(0);
        glVertexAttribPointer
        (
            0,
            4,
            GL_FLOAT,
            false,
            4*sizeof(float),
            0
        );
        glVertexAttribDivisor(0,0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~glCompute()
    {
        glDeleteTextures(attributes.size(), textures.data());
        glDeleteFramebuffers(1, &frameBuffer);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    void set(std::string attribute, std::vector<float> newData)
    {
        if (attributes.find(attribute) != attributes.end())
        {
            if (newData.size() <= attributes[attribute].data.size())
            {
                std::copy
                (
                    newData.begin(),
                    newData.end(),
                    attributes[attribute].data.begin()
                );
            }
        }
    }

    void set(std::string attribute, float datum, uint64_t index)
    {
        if (attributes.find(attribute) != attributes.end())
        {
            if (index <= attributes[attribute].data.size())
            {
                attributes[attribute].data[index] = datum;
            }
        }
    }

    std::vector<float> & get(std::string attribute)
    {
        if (attributes.find(attribute) != attributes.end())
        {
            return attributes[attribute].data;
        }
        throw std::runtime_error("No attribute: "+attribute);
    }

    const std::vector<float> & result() { return output; }

    void sync(std::string attribute)
    {
        if (attributes.find(attribute) != attributes.end())
        {
            Attribute & attr = attributes[attribute];
            transferToTexture2DR32F(attr.texture, attr.data, attr.dimX, attr.dimY);
        }
    }

    void sync()
    {
        for (const auto & attr : attributes)
        {
            sync(attr.first);
        }
    }

    void compute(bool syncResult)
    {
        shader.use();

        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textures.back());

        glFramebufferTexture2D
        (
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            textures.back(),
            0
        );
        GLenum drawBuffers[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, drawBuffers);

        GLuint t = 0;
        for (const auto & attr : attributes)
        {
            glActiveTexture(GL_TEXTURE0+t+2);
            glBindTexture(GL_TEXTURE_2D, attr.second.texture);
            shader.setUniform(attr.first, Sampler2D(t+2));
            t++;
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        glDepthMask(false);
        glDisable(GL_BLEND);
        glViewport(0, 0, outputSize.first, outputSize.second);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (syncResult)
        {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textures.back());
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, output.data());
        }

        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindVertexArray(0);
    }

    const std::vector<float> & syncResult()
    {
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels
        (
            0,
            0,
            outputSize.first,
            outputSize.second,
            GL_RED,
            GL_FLOAT,
            output.data()
        );
        return result();
    }

private:

    struct Attribute
    {
        Attribute() = default;

        Attribute
        (
            std::vector<float> data,
            GLuint texture,
            uint64_t dimX,
            uint64_t dimY
        )
        : data(data), texture(texture), dimX(dimX), dimY(dimY)
        {}

        std::vector<float> data;
        GLuint texture;
        uint64_t dimX;
        uint64_t dimY;
    };

    std::vector<GLuint> textures;
    std::map<std::string, Attribute> attributes;

    std::vector<float> output;
    std::pair<uint64_t, uint64_t> outputSize;
    GLuint frameBuffer, vao, vbo;

    float quad[6*4] =
    {
        -1.0, -1.0, 0.0, 0.0,
         1.0, -1.0, 1.0, 0.0,
         1.0,  1.0, 1.0, 1.0,
        -1.0, -1.0, 0.0, 0.0,
        -1.0,  1.0, 0.0, 1.0,
         1.0,  1.0, 1.0, 1.0
    };
};
#endif /* GLGPGPU_H */
