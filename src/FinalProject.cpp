/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <Shader.h>
#include "stb_image.h"

#include "Camera/camera.h"
#include "Bezier/bezier.h"

#include "nlohmann/json.hpp"
using json = nlohmann::json;


struct Light {
    glm::vec3 ka;
    glm::vec3 kd;
    glm::vec3 ks;
    glm::vec3 ke;
    float q;
};

struct Geometry
{
    GLuint VAO;
    GLuint vertexCount;
    GLuint textureID = 0;
    glm::vec3 position;
    glm::vec3 scale = glm::vec3(0.2f, 0.2f, 0.2f);
    string textureFilePath;
    Light light;

    std::vector<glm::vec3> trajectoryPoints;
    int currentTarget = 0;
    float progress = 0.0f;
    float movementSpeed = 1.0f;
    string name;
};

std::vector<glm::vec3> generateCircleControlPointsSet(int nPoints, float radius);
void generateControlPointsSet(Geometry& geometry);
void updateTrajectory(Geometry& obj, float deltaTime);

// Protótipo da função de callback de mouse
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
Geometry setupGeometry(const char* filepath);
GLuint setupBackgroundQuad(GLuint& VAO, GLuint& VBO, const std::string& texturePath);

bool loadObject(
    const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals);
int loadTexture(const string& path);
Light loadMTL(const string& path, string& texturePath);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX=false, rotateY=false, rotateZ=false;

glm::vec3 lightPos(2.0f, 2.0f, 1.0f);
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);

Camera camera(glm::vec3(0.0f, 0.0f, 10.0f),
              glm::vec3(0.0f, 1.0f, 0.0f),
              -90.0f, 0.0f);


float deltaTime = 0.0f; // tempo entre frames
float lastFrame = 0.0f;

bool firstMouse = true;
float yaw = -90.0f;   // olhando para -Z inicialmente
float pitch = 0.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
float fov = 45.0f;  // para zoom no próximo passo

int idSelect = 0;

std::vector<Geometry> sceneObjects;

string filePath = "";

void updateCameraPos(GLFWwindow *window) {
    float cameraSpeed = 2.5f * deltaTime; // quanto mais tempo passou, mais rápido

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void generateControlPointsSet(Geometry& geometry)
{
    if(geometry.name == "moon") {
        geometry.trajectoryPoints = generateCircleControlPointsSet(50, 0.8f);
        geometry.movementSpeed = 0.5f;
    }

    else if(geometry.name == "earth") {
        geometry.trajectoryPoints = generateCircleControlPointsSet(50, 3.0f);
        geometry.movementSpeed = 0.2f;
    }

    else if (geometry.name == "satellite") {
        std::vector<glm::vec3> controlPoints = {
            glm::vec3(1.5f, 0.0f, -1.0f),
            glm::vec3(4.0f, 5.0f, -1.0f),
            glm::vec3(-4.0f, 5.0f, -1.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(4.0f, -5.0f, -1.0f),
            glm::vec3(-4.0f, -5.0f, -1.0f),
            glm::vec3(1.5f, 0.0f, -1.0f)
        };

        geometry.trajectoryPoints = generateCubicBezierCurve(controlPoints, 100);
        std::cout << "Pontos gerados para o satellite: " << geometry.trajectoryPoints.size() << std::endl;

        geometry.movementSpeed = 0.3f;
    }
}

json loadConfig(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file) {
        std::cerr << "Erro ao abrir o arquivo de configuração: " << filepath << std::endl;
        exit(EXIT_FAILURE);
    }
    json config;
    file >> config;
    return config;
}

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

    json config = loadConfig("../finalProject/config/project-config.json");

    std::cout << "Conteúdo do arquivo JSON carregado:\n" << config["objects"][0]["objFile"] << std::endl;

	// Criação da janela GLFW
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Universe - Parametric Curves", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);


	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;

	}

	// Obtendo as informações de versão
	const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;


	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
    
	Shader shader("../finalProject/shaders/vertex-shader.glsl", "../finalProject/shaders/fragment-shader.glsl");
    
    Geometry earth = setupGeometry(config["objects"][0]["objFile"].get<string>().c_str());
    earth.name = config["objects"][0]["name"];
    generateControlPointsSet(earth);
    earth.position = earth.trajectoryPoints[0];
    earth.scale = glm::vec3(
        config["objects"][0]["scale"][0],
        config["objects"][0]["scale"][1],
        config["objects"][0]["scale"][2]
    );

    Geometry moon = setupGeometry(config["objects"][1]["objFile"].get<string>().c_str());
    moon.name = config["objects"][1]["name"];
    generateControlPointsSet(moon);
    moon.position = moon.trajectoryPoints[0];
    moon.scale = glm::vec3(
        config["objects"][1]["scale"][0],
        config["objects"][1]["scale"][1],
        config["objects"][1]["scale"][2]
    );

    Geometry sun = setupGeometry(config["objects"][2]["objFile"].get<string>().c_str());
    sun.name = config["objects"][2]["name"];
    sun.position = glm::vec3(0.0, 0.0, 0.0);
    sun.scale = glm::vec3(
        config["objects"][2]["scale"][0],
        config["objects"][2]["scale"][1],
        config["objects"][2]["scale"][2]
    );

    Geometry satellite = setupGeometry(config["objects"][3]["objFile"].get<string>().c_str());
    satellite.name = config["objects"][3]["name"];
    generateControlPointsSet(satellite);
    satellite.position = satellite.trajectoryPoints[0];
    satellite.scale = glm::vec3(
        config["objects"][3]["scale"][0],
        config["objects"][3]["scale"][1],
        config["objects"][3]["scale"][2]
    );

    sceneObjects = {earth, moon, sun, satellite};
	glUseProgram(shader.ID);

	glm::mat4 model = glm::mat4(1); //matriz identidade;
	GLint modelLoc = glGetUniformLocation(shader.ID, "model");

    // Ilumincação partindo do sol
    shader.setVec3("lightPos", sun.position.x, sun.position.y, sun.position.z);
    shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

    glm::mat4 view = camera.GetViewMatrix();
    shader.setMat4("view", glm::value_ptr(view));

    camera.Fov = config["camera"]["fov"];
    camera.Yaw = config["camera"]["yaw"];
    camera.Pitch = config["camera"]["pitch"];
    float near = config["camera"]["frustum"]["near"];
    float far = config["camera"]["frustum"]["far"];
    
    glm::mat4 projection = glm::perspective(glm::radians(camera.Fov), (float)width / height, near, far);
   
    
    shader.setMat4("projection", glm::value_ptr(projection));


	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    GLuint backgroundVAO, backgroundVBO;
    GLuint backgroundTexture = setupBackgroundQuad(backgroundVAO, backgroundVBO, "../assets/Background/blue-universe-bg.jpg");
    Shader backgroundShader("../finalProject/shaders/vertex-bg.glsl", "../finalProject/shaders/fragment-bg.glsl");

    if (backgroundTexture == 0) {
        std::cerr << "Erro: textura de background não carregada corretamente!" << std::endl;
    }

	glEnable(GL_DEPTH_TEST);


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f); //cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		float angle = (GLfloat)glfwGetTime();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        updateCameraPos(window);

        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", glm::value_ptr(view));

        glDisable(GL_DEPTH_TEST);
        glUseProgram(backgroundShader.ID);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, backgroundTexture);
        glUniform1i(glGetUniformLocation(backgroundShader.ID, "background"), 0);

        if (backgroundShader.ID == 0) {
            std::cerr << "Erro ao compilar o shader de background!" << std::endl;
        }

        glBindVertexArray(backgroundVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        glUseProgram(shader.ID);
        GLint modelLoc = glGetUniformLocation(shader.ID, "model");

        updateTrajectory(sceneObjects[0], deltaTime); // Atualiza a Terra
        updateTrajectory(sceneObjects[1], deltaTime); // Atualiza a Lua
        updateTrajectory(sceneObjects[3], deltaTime); // Atualiza o satellite

        for (size_t i = 0; i < sceneObjects.size(); ++i)
        {
            Geometry& geometry = sceneObjects[i];
            glm::mat4 model = glm::mat4(1.0f);

            if (geometry.name == "sun")
            {
                model = glm::translate(model, geometry.position);

                  if (rotateX && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                else if (rotateY && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                else if (rotateZ && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

                model = glm::scale(model, geometry.scale);
            }
            else if (geometry.name == "satellite")
            {
                model = glm::translate(model, geometry.position);

                if (rotateX && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                else if (rotateY && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                else if (rotateZ && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

                model = glm::scale(model, geometry.scale);
            }
            else if (geometry.name == "earth")
            {
                updateTrajectory(geometry, deltaTime);
                model = glm::translate(model, geometry.position);

                if (rotateX && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                else if (rotateY && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                else if (rotateZ && i == idSelect)
                    model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

                model = glm::scale(model, geometry.scale);
            }
            else if (geometry.name == "moon")
            {
                Geometry& earth = sceneObjects[0];

                updateTrajectory(earth, deltaTime);
                updateTrajectory(geometry, deltaTime);

                glm::mat4 earthModel = glm::mat4(1.0f);
                earthModel = glm::translate(earthModel, earth.position);

                glm::mat4 moonModel = glm::mat4(1.0f);
                moonModel = glm::translate(moonModel, geometry.position);

                if (rotateX && i == idSelect)
                    moonModel = glm::rotate(moonModel, angle, glm::vec3(1.0f, 0.0f, 0.0f));
                else if (rotateY && i == idSelect)
                    moonModel = glm::rotate(moonModel, angle, glm::vec3(0.0f, 1.0f, 0.0f));
                else if (rotateZ && i == idSelect)
                    moonModel = glm::rotate(moonModel, angle, glm::vec3(0.0f, 0.0f, 1.0f));

                moonModel = glm::scale(moonModel, geometry.scale);

                model = earthModel * moonModel;
            }

            glm::vec3 lightPosView = glm::vec3(view * glm::vec4(sceneObjects[2].position, 1.0f));
            shader.setVec3("lightPos", lightPosView.x, lightPosView.y, lightPosView.z);
            shader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);
            shader.setVec3("ka", geometry.light.ka.r, geometry.light.ka.g, geometry.light.ka.b);
            shader.setVec3("kd", geometry.light.kd.r, geometry.light.kd.g, geometry.light.kd.b);
            shader.setVec3("ks", geometry.light.ks.r, geometry.light.ks.g, geometry.light.ks.b);
            shader.setVec3("ke", geometry.light.ke.r, geometry.light.ke.g, geometry.light.ke.b);
            shader.setFloat("q", geometry.light.q);

            shader.setMat4("model", glm::value_ptr(model));
            glBindTexture(GL_TEXTURE_2D, geometry.textureID);
            glBindVertexArray(geometry.VAO);
            glDrawArrays(GL_TRIANGLES, 0, geometry.vertexCount);
        }

        updateCameraPos(window);

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	for (const Geometry& obj : sceneObjects)
    {
        glDeleteVertexArrays(1, &obj.VAO);
    }
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

std::vector<glm::vec3> generateCircleControlPointsSet(int nPoints, float radius)
{
    std::vector<glm::vec3> controlPoints;

    for (int i = 0; i < nPoints; i++)
    {
        float angle = (2.0f * M_PI * i) / nPoints;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        controlPoints.push_back(glm::vec3(x, y, 0.0f));
    }

    return controlPoints;
}


void updateTrajectory(Geometry& obj, float deltaTime) {
    if (obj.trajectoryPoints.size() < 2)
    return;

    glm::vec3 currentPos = obj.position;
    glm::vec3 targetPos = obj.trajectoryPoints[obj.currentTarget];

    float distance = glm::distance(currentPos, targetPos);
    float step = obj.movementSpeed * deltaTime;

    if (distance < 0.01f) {
        // Chegou no ponto atual, ir para o próximo
        obj.currentTarget = (obj.currentTarget + 1) % obj.trajectoryPoints.size();
    } else {
        // Mover em direção ao próximo ponto
        glm::vec3 direction = glm::normalize(targetPos - currentPos);
        obj.position += direction * step;

        // Evitar ultrapassar o ponto
        if (glm::distance(obj.position, targetPos) < step) {
            obj.position = targetPos;
            obj.currentTarget = (obj.currentTarget + 1) % obj.trajectoryPoints.size();
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // y invertido
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		rotateX = true;
		rotateY = false;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Y && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = true;
		rotateZ = false;
	}

	if (key == GLFW_KEY_Z && action == GLFW_PRESS)
	{
		rotateX = false;
		rotateY = false;
		rotateZ = true;
	}

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		idSelect++;
		if(idSelect > sceneObjects.size() - 1)
		{
			idSelect = 0;
		}
	}


	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		if (idSelect == 0)
		{
			idSelect = sceneObjects.size() - 1;  // vai para o último
		}
		else
		{
			idSelect--;
		}
	}


    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        lightColor = glm::vec3(1.0f, 0.0f, 0.85f); // rosinha
    }

    if (key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        lightColor = glm::vec3(1.0f, 1.0f, 1.0f); // padrão
    }
}


// Esta função está bastante harcoded - objetivo é criar os buffers que armazenam a 
// geometria de um triângulo
// Apenas atributo coordenada nos vértices
// 1 VBO com as coordenadas, VAO com apenas 1 ponteiro para atributo
// A função retorna o identificador do VAO
Geometry setupGeometry(const char* filepath)
{
    std::vector<GLfloat> vertices;
	std::vector<glm::vec3> v;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    if (!loadObject(filepath, v, uvs, normals))
    {
        std::cerr << "Failed to load object geometry" << std::endl;
        return {};
    }

    
    vertices.reserve(v.size() * 8); // 3 posição + 3 cor + 2 UV
    
    for (size_t i = 0; i < v.size(); ++i)
    {
        vertices.insert(vertices.end(), {
            v[i].x, v[i].y, v[i].z,     // posição
             normals[i].x, normals[i].y, normals[i].z,   // normal      
            uvs[i].x, uvs[i].y  // coordenadas de textura
        });
    }
    
    GLuint VAO, VBO;

	//Geração do identificador do VAO (Vertex Array Object)
	glGenVertexArrays(1, &VAO);

	// Vincula (bind) o VAO primeiro, e em seguida  conecta e seta o(s) buffer(s) de vértices
	// e os ponteiros para os atributos 
	glBindVertexArray(VAO);

	//Geração do identificador do VBO
	glGenBuffers(1, &VBO);
	//Faz a conexão (vincula) do buffer como um buffer de array
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
	
    // posição (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal (location = 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UV (location = 2) 
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Geometry geometry;
    geometry.VAO = VAO;
    geometry.vertexCount = vertices.size() / 8;

	string basePath = string(filepath).substr(0, string(filepath).find_last_of("/"));
    string filenameNoExt = string(filepath).substr(string(filepath).find_last_of("/") + 1);
    filenameNoExt = filenameNoExt.substr(0, filenameNoExt.find_last_of(".")); // remove extensão

    string mtlPath = basePath + "/" + filenameNoExt + ".mtl";
    string textureFile;
    geometry.light = loadMTL(mtlPath, textureFile);

    if (!textureFile.empty())
    {
        string fullTexturePath = basePath + "/" + textureFile;
        geometry.textureID = loadTexture(fullTexturePath);
        geometry.textureFilePath = fullTexturePath;
    }

    return geometry;
}

GLuint setupBackgroundQuad(GLuint& VAO, GLuint& VBO, const std::string& texturePath)
{
    float quadVertices[] = {
        // posições    // UVs
        -1.0f,  1.0f,  0.0f, 1.0f, // canto superior esquerdo
        -1.0f, -1.0f,  0.0f, 0.0f, // canto inferior esquerdo
         1.0f, -1.0f,  1.0f, 0.0f, // canto inferior direito

        -1.0f,  1.0f,  0.0f, 1.0f, // canto superior esquerdo
         1.0f, -1.0f,  1.0f, 0.0f, // canto inferior direito
         1.0f,  1.0f,  1.0f, 1.0f  // canto superior direito
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Posição (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // UV (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return loadTexture(texturePath);
}


bool loadObject(
    const char* path,
    std::vector<glm::vec3>& out_vertices,
    std::vector<glm::vec2>& out_uvs,
    std::vector<glm::vec3>& out_normals)
{
    std::ifstream file(path);

    if (!file)
    {
        std::cerr << "Failed to open file: " << path << std::endl;
        return false;
    }

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::string line;

    while (std::getline(file, line))
    {
        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") // vertex data
        {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            temp_vertices.push_back(vertex);
        }
        else if (type == "vt") // texture coordinate (UV)
        {
            glm::vec2 uv;
            iss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (type == "vn") // normal vector
        {
            glm::vec3 normal;
            iss >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (type == "f")
        {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            char slash;

            for (int i = 0; i < 3; ++i)
            {
                iss >> vertexIndex[i] >> slash >> uvIndex[i] >> slash >> normalIndex[i];
                vertexIndices.push_back(vertexIndex[i]);
                uvIndices.push_back(uvIndex[i]);
                normalIndices.push_back(normalIndex[i]);
            }
        }
    }

    for (unsigned int i = 0; i < vertexIndices.size(); ++i)
    {
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int uvIndex = uvIndices[i];
        unsigned int normalIndex = normalIndices[i];

        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec2 uv = temp_uvs[uvIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        out_vertices.push_back(vertex);
        out_uvs.push_back(uv);
        out_normals.push_back(normal);
    }

    file.close();

    return true;
}

int loadTexture(const string& path)
{
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    std::cout << "Tentando carregar textura de: " << path << std::endl;

    if (!data) {
        std::cout << "Failed to load texture: " << path << std::endl;
        glDeleteTextures(1, &texID);
        return 0;  // textura inválida
    }

    if (data)
    {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

Light loadMTL(const string& path, string& texturePath)
{
    ifstream mtlFile(path);
    if (!mtlFile)
    {
        cerr << "Failed to open MTL file: " << path << endl;
        return {};
    }

    Light light;
    string line;
    while (getline(mtlFile, line))
    {
        istringstream iss(line);
        string keyword;
        iss >> keyword;

        if (keyword == "Ka") {
            iss >> light.ka.r >> light.ka.g >> light.ka.b;
        } else if (keyword == "Kd") {
            iss >> light.kd.r >> light.kd.g >> light.kd.b;
        } else if (keyword == "Ks") {
            iss >> light.ks.r >> light.ks.g >> light.ks.b;
        }  else if (keyword == "Ke")
        {
            iss >> light.ke.r >> light.ke.g >> light.ke.b;
        } else if (keyword == "Ns") {
            iss >> light.q;
        } else if (keyword == "map_Kd") {
            iss >> texturePath;
        }
    }

    mtlFile.close();

    if (texturePath.empty())
    {
        cerr << "No diffuse texture found in MTL file: " << path << endl;
    }
    return light;
}
