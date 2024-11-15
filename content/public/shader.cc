// Copyright 2024 Admenri.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/public/shader.h"

#include "base/exceptions/exception.h"
#include "content/public/bitmap.h"

namespace content {

namespace {

void MultiplyMatrices(const GLfloat* matrixA,
                      const GLfloat* matrixB,
                      GLfloat* result) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      result[i + j * 4] = 0;
      for (int k = 0; k < 4; ++k) {
        result[i + j * 4] += matrixA[i + k * 4] * matrixB[k + j * 4];
      }
    }
  }
}

}  // namespace

Shader::Shader(scoped_refptr<Graphics> screen)
    : GraphicElement(screen),
      Disposable(screen.get()),
      vertex_shader_(0),
      frag_shader_(0),
      program_(0),
      equal_mode_(GL_FUNC_ADD),
      srcRGB_(GL_SRC_ALPHA),
      dstRGB_(GL_ONE_MINUS_SRC_ALPHA),
      srcAlpha_(GL_ONE),
      dstAlpha_(GL_ONE_MINUS_SRC_ALPHA) {}

Shader::~Shader() {
  Dispose();
}

void Shader::Compile(const std::string& vertex_shader,
                     const std::string& fragment_shader) {
  location_cache_.clear();
  bind_textures_.clear();

  screen()->renderer()->PostTask(
      base::BindOnce(&Shader::CompileInternal, base::Unretained(this),
                     vertex_shader, fragment_shader));
  screen()->renderer()->WaitForSync();
}

void Shader::Reset() {
  location_cache_.clear();
  bind_textures_.clear();

  screen()->renderer()->PostTask(
      base::BindOnce(&Shader::ResetInternal, base::Unretained(this)));
  screen()->renderer()->WaitForSync();
}

void Shader::SetBlend(GLenum mode,
                      GLenum srcRGB,
                      GLenum dstRGB,
                      GLenum srcAlpha,
                      GLenum dstAlpha) {
  equal_mode_ = mode;
  srcRGB_ = srcRGB;
  dstRGB_ = dstRGB;
  srcAlpha_ = srcAlpha;
  dstAlpha_ = dstAlpha;
}

void Shader::SetParam(const std::string& uniform,
                      const std::vector<float>& params,
                      int count) {
  screen()->renderer()->PostTask(base::BindOnce(&Shader::SetParam1Internal,
                                                base::Unretained(this), uniform,
                                                params, count));
}

void Shader::SetParam(const std::string& uniform,
                      const std::vector<int>& params,
                      int count) {
  screen()->renderer()->PostTask(base::BindOnce(&Shader::SetParam2Internal,
                                                base::Unretained(this), uniform,
                                                params, count));
}

void Shader::SetParam(const std::string& uniform,
                      const std::vector<float>& matrix,
                      int count,
                      bool transpose) {
  screen()->renderer()->PostTask(base::BindOnce(&Shader::SetParam3Internal,
                                                base::Unretained(this), uniform,
                                                matrix, count, transpose));
}

void Shader::SetParam(const std::string& uniform,
                      scoped_refptr<Bitmap> texture,
                      int index) {
  screen()->renderer()->PostTask(base::BindOnce(&Shader::SetParam4Internal,
                                                base::Unretained(this), uniform,
                                                texture, index));
}

void Shader::OnObjectDisposed() {
  location_cache_.clear();
  bind_textures_.clear();

  screen()->renderer()->PostTask(base::BindOnce(
      [](GLuint vertex_shader, GLuint frag_shader, GLuint program) {
        if (vertex_shader)
          renderer::GL.DeleteShader(vertex_shader);
        if (frag_shader)
          renderer::GL.DeleteShader(frag_shader);
        if (program)
          renderer::GL.DeleteProgram(program);
      },
      vertex_shader_, frag_shader_, program_));
}

void Shader::CompileInternal(const std::string& vertex_shader,
                             const std::string& fragment_shader) {
  if (vertex_shader_)
    renderer::GL.DeleteShader(vertex_shader_);
  if (frag_shader_)
    renderer::GL.DeleteShader(frag_shader_);
  if (program_)
    renderer::GL.DeleteProgram(program_);
  vertex_shader_ = renderer::GL.CreateShader(GL_VERTEX_SHADER);
  frag_shader_ = renderer::GL.CreateShader(GL_FRAGMENT_SHADER);
  program_ = renderer::GL.CreateProgram();

  auto compile_shader = [&](GLuint glshader, const std::string& shader_source) {
    std::vector<const GLchar*> shader_srcs;
    std::vector<GLint> shader_sizes;

    // Setup GLSL header
    if (renderer::GSM.glsl_es()) {
      shader_srcs.push_back(renderer::kGLSLESHeader);
      shader_sizes.push_back(strlen(renderer::kGLSLESHeader));
    } else {
      shader_srcs.push_back(renderer::kGLSLDesktopHeader);
      shader_sizes.push_back(strlen(renderer::kGLSLDesktopHeader));
    }

    // Setup shader source
    shader_srcs.push_back(
        reinterpret_cast<const GLchar*>(shader_source.c_str()));
    shader_sizes.push_back(static_cast<GLint>(shader_source.size()));

    // Setup shader program
    renderer::GL.ShaderSource(glshader,
                              static_cast<GLsizei>(shader_srcs.size()),
                              shader_srcs.data(), shader_sizes.data());

    renderer::GL.CompileShader(glshader);
    GLint success;
    renderer::GL.GetShaderiv(glshader, GL_COMPILE_STATUS, &success);
    if (!success) {
      GLint log_length;
      renderer::GL.GetShaderiv(glshader, GL_INFO_LOG_LENGTH, &log_length);

      std::string log(log_length, '\0');
      renderer::GL.GetShaderInfoLog(glshader, static_cast<GLsizei>(log.size()),
                                    0, log.data());
      LOG(WARNING) << "[Shader::Compile] ShaderCompile: " << log;

      return false;
    }

    return true;
  };

  if (!compile_shader(vertex_shader_, vertex_shader))
    return;
  if (!compile_shader(frag_shader_, fragment_shader))
    return;

  // Link program
  LinkProgramInternal();
}

void Shader::ResetInternal() {
  if (program_)
    renderer::GL.DeleteProgram(program_);
  program_ = renderer::GL.CreateProgram();

  // Link program
  LinkProgramInternal();
}

void Shader::LinkProgramInternal() {
  renderer::GL.AttachShader(program_, vertex_shader_);
  renderer::GL.AttachShader(program_, frag_shader_);

  renderer::GL.BindAttribLocation(
      program_, renderer::GLES2Shader::AttribLocation::Position, "a_position");
  renderer::GL.BindAttribLocation(
      program_, renderer::GLES2Shader::AttribLocation::TexCoord, "a_texCoord");
  renderer::GL.BindAttribLocation(
      program_, renderer::GLES2Shader::AttribLocation::Color, "a_color");

  renderer::GL.LinkProgram(program_);

  GLint success;
  renderer::GL.GetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success) {
    GLint log_length;
    renderer::GL.GetProgramiv(program_, GL_INFO_LOG_LENGTH, &log_length);

    std::string log(log_length, '\0');
    renderer::GL.GetProgramInfoLog(program_, static_cast<GLsizei>(log.size()),
                                   0, log.data());

    LOG(WARNING) << "[Shader::Compile] Program: " << log;
  }
}

void Shader::SetParam1Internal(const std::string& uniform,
                               const std::vector<float>& params,
                               int count) {
  BindShader();

  GLint uniform_location = GetUniformLocation(uniform);
  if (!count) {
    // Single Vec
    switch (params.size()) {
      case 1:
        renderer::GL.Uniform1f(uniform_location, params[0]);
        break;
      case 2:
        renderer::GL.Uniform2f(uniform_location, params[0], params[1]);
        break;
      case 3:
        renderer::GL.Uniform3f(uniform_location, params[0], params[1],
                               params[2]);
        break;
      case 4:
        renderer::GL.Uniform4f(uniform_location, params[0], params[1],
                               params[2], params[3]);
        break;
      default:
        LOG(WARNING) << "[Shader::SetParam] unexpected float params size: "
                     << params.size();
        break;
    }
  } else {
    // Array Vec
    int array_size = params.size() / count;
    switch (count) {
      case 1:
        renderer::GL.Uniform1fv(uniform_location, array_size, params.data());
        break;
      case 2:
        renderer::GL.Uniform2fv(uniform_location, array_size, params.data());
        break;
      case 3:
        renderer::GL.Uniform3fv(uniform_location, array_size, params.data());
        break;
      case 4:
        renderer::GL.Uniform4fv(uniform_location, array_size, params.data());
        break;
      default:
        LOG(WARNING)
            << "[Shader::SetParam] invalid float Vec array element size: "
            << count << " (1, 2, 3, 4).";
        break;
    }
  }
}

void Shader::SetParam2Internal(const std::string& uniform,
                               const std::vector<int>& params,
                               int count) {
  BindShader();

  GLint uniform_location = GetUniformLocation(uniform);
  if (!count) {
    // Single Vec
    switch (params.size()) {
      case 1:
        renderer::GL.Uniform1i(uniform_location, params[0]);
        break;
      case 2:
        renderer::GL.Uniform2i(uniform_location, params[0], params[1]);
        break;
      case 3:
        renderer::GL.Uniform3i(uniform_location, params[0], params[1],
                               params[2]);
        break;
      case 4:
        renderer::GL.Uniform4i(uniform_location, params[0], params[1],
                               params[2], params[3]);
        break;
      default:
        LOG(WARNING) << "[Shader::SetParam] unexpected integer params size: "
                     << params.size();
        break;
    }
  } else {
    // Array Vec
    int array_size = params.size() / count;
    switch (count) {
      case 1:
        renderer::GL.Uniform1iv(uniform_location, array_size, params.data());
        break;
      case 2:
        renderer::GL.Uniform2iv(uniform_location, array_size, params.data());
        break;
      case 3:
        renderer::GL.Uniform3iv(uniform_location, array_size, params.data());
        break;
      case 4:
        renderer::GL.Uniform4iv(uniform_location, array_size, params.data());
        break;
      default:
        LOG(WARNING)
            << "[Shader::SetParam] invalid integer Vec array element size: "
            << count << " (1, 2, 3, 4).";
        break;
    }
  }
}

void Shader::SetParam3Internal(const std::string& uniform,
                               const std::vector<float>& matrix,
                               int count,
                               bool transpose) {
  BindShader();

  GLint uniform_location = GetUniformLocation(uniform);
  int matrix_size = matrix.size() / (count * count);
  switch (count) {
    case 2:
      renderer::GL.UniformMatrix2fv(uniform_location, matrix_size, transpose,
                                    matrix.data());
      break;
    case 3:
      renderer::GL.UniformMatrix3fv(uniform_location, matrix_size, transpose,
                                    matrix.data());
      break;
    case 4:
      renderer::GL.UniformMatrix4fv(uniform_location, matrix_size, transpose,
                                    matrix.data());
      break;
    default:
      LOG(WARNING) << "[Shader::SetParam] invalid matrix size: " << count
                   << " (2, 3, 4).";
      break;
  }
}

void Shader::SetParam4Internal(const std::string& uniform,
                               scoped_refptr<Bitmap> texture,
                               int index) {
  const GLint location = GetUniformLocation(uniform);
  auto it = bind_textures_.find(index);
  if (it != bind_textures_.end()) {
    it->second.texture = texture;
    it->second.location = location;
  } else {
    TextureUnit tex_unit;
    tex_unit.texture = texture;
    tex_unit.location = location;
    bind_textures_.emplace(index, std::move(tex_unit));
  }
}

void Shader::BindShader() {
  renderer::GSM.states.program.Set(program_);
}

void Shader::SetInternalUniform() {
  // Apply viewport projection
  base::Vec2 viewport_size = renderer::GSM.states.viewport.Current().Size();
  GLint projection_location = GetUniformLocation("u_projectionMat");
  const float a = 2.0f / viewport_size.x;
  const float b = 2.0f / viewport_size.y;
  const float c = -2.0f;

  GLfloat mat[] = {a, 0, 0, 0, 0, b, 0, 0, 0, 0, c, 0, -1, -1, -1, 1};
  renderer::GL.UniformMatrix4fv(projection_location, 1, GL_FALSE, mat);

  // Apply bind texture unit
  for (auto& it : bind_textures_) {
    auto& tex_unit = it.second;
    if (tex_unit.location >= 0 && IsObjectValid(tex_unit.texture.get()))
      renderer::GLES2ShaderBase::SetTexture(
          tex_unit.location, tex_unit.texture->GetRaw()->tex.gl, it.first);
  }
}

void Shader::SetBlendFunc() {
  renderer::GL.BlendEquation(equal_mode_);
  renderer::GL.BlendFuncSeparate(srcRGB_, dstRGB_, srcAlpha_, dstAlpha_);
}

GLint Shader::GetUniformLocation(const std::string& name) {
  auto it = location_cache_.find(name);
  if (it != location_cache_.end())
    return it->second;

  GLint location = renderer::GL.GetUniformLocation(program_, name.c_str());
  location_cache_.insert({name, location});

  return location;
}

}  // namespace content
