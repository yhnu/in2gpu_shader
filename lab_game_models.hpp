#if !defined(_LAB_GAME_MODElS_H)
#define _LAB_GAME_MODElS_H

#pragma once
#include "dependente/glew/glew.h"
#include "dependente/freeglut/freeglut.h"
#include <vector>
#include <map>

namespace lab
{
struct VertexFormat
{
  glm::vec3 position; //our first vertex attribute

  VertexFormat(const glm::vec3 &pos)
  {
    position = pos;
  }
};

//I explain this structure in Part III
struct Model
{
  unsigned int vao;
  std::vector<unsigned int> vbos;

  Model() {}
};

class GameModels
{
public:
  GameModels()
  {
  }

  ~GameModels()
  {
  }

  void CreateTriangleModel(const std::string &gameModelName)
  {
    unsigned int vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //triangle vertices on stack
    std::vector<VertexFormat> vertices;
    //specify vertex positions
    vertices.push_back(VertexFormat(glm::vec3(-0.5, -0.5, 0.0)));
    vertices.push_back(VertexFormat(glm::vec3(0.0, 0.5, 0.0)));
    vertices.push_back(VertexFormat(glm::vec3(0.5, -0.5, 0.0)));

    unsigned int vbo; //our vbo handler
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo); //bind to context
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexFormat) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void *)0);

    Model myModel;                          //is allocated on Stack
    myModel.vao = vao;                      //add vao


    
    myModel.vbos.push_back(vbo);            //add vbo
    GameModelList[gameModelName] = myModel; //add to std::map
  }

  void DeleteModel(const std::string &gameModelName)
  {
    odel model = GameModelList[gameModelName];
    unsigned int p = model.vao;
    glDeleteVertexArrays(1, &p);
    glDeleteBuffers(model.vbos.size(), &model.vbos[0]);
    model.vbos.clear();
    GameModelList.erase(gameModelName);
  }

  unsigned int GetModel(const std::string &gameModelName)
  {
    return GameModelList[gameModelName].vao;
  }

private:
  std::map<std::string, Model> GameModelList; //keep our models
};
} // namespace lab

#endif // _LAB_GAME_MODElS_H