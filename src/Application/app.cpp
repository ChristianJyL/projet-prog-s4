#include "app.hpp"
#include <imgui.h>
#include <iostream>
#include <string>
#include "Chess/Board.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h> // Ajoutez cette ligne

void app::init()
{
    m_board.initializeBoard(); // Initialize the chessboard
    m_renderer3D.initialize(); // Initialize the 3D renderer
}

void app::update()
{
    // Créer une section pour le rendu 3D
    ImGui::Begin("3D Chess Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
    
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    // Vérifier si la taille de la fenêtre est valide
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        // Créer une fenêtre de rendu OpenGL
        // Obtenir la position de la fenêtre ImGui dans l'écran
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentPos = ImGui::GetCursorScreenPos();
        
        // Position fixe de la caméra (sans rotation)
        glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 3.0f);
        
        // Créer des matrices de vue et de projection
        glm::mat4 view = glm::lookAt(
            cameraPos,                   // Position fixe de la caméra
            glm::vec3(0.0f, 0.0f, 0.0f), // Point visé
            glm::vec3(0.0f, 1.0f, 0.0f)  // Vecteur "up"
        );
        
        glm::mat4 projection = glm::perspective(
            glm::radians(60.0f),                    // FOV
            viewportSize.x / std::max(viewportSize.y, 1.0f), // Aspect ratio
            0.1f,                                   // Near clipping plane
            100.0f                                  // Far clipping plane
        );

        // Mettre à jour le renderer 3D sans l'animation
        m_renderer3D.setCameraPosition(cameraPos);

        // Créer un framebuffer de rendu si nécessaire
        static GLuint fbo = 0, renderTexture = 0, depthBuffer = 0;
        if (fbo == 0) {
            // Créer un framebuffer pour le rendu
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            
            // Créer une texture pour y rendre
            glGenTextures(1, &renderTexture);
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
            
            // Créer un renderbuffer pour la profondeur
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        // Redimensionner le framebuffer si la taille de la fenêtre a changé
        static ImVec2 lastSize = viewportSize;
        if (lastSize.x != viewportSize.x || lastSize.y != viewportSize.y) {
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            
            lastSize = viewportSize;
        }
        
        // Rendre dans le framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, (int)viewportSize.x, (int)viewportSize.y);
        
        // Effacer le buffer
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Dessiner la skybox dans le framebuffer
        if (m_renderer3D.isInitialized()) {
            try {
                m_renderer3D.getSkybox().Draw(view, projection);
            } catch (const std::exception& e) {
                std::cerr << "Erreur lors du dessin de la skybox: " << e.what() << std::endl;
            }
        }
        
        // Retourner au framebuffer par défaut
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Afficher la texture rendue dans ImGui
        ImGui::Image((void*)(intptr_t)renderTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
    }
    
    ImGui::End();
    
    // Afficher l'échiquier
    m_board.drawBoard();

    // Afficher un message si la partie est terminée
    if (m_board.isGameOver())
    {
        ImGui::OpenPopup("Game Over");
    }

    // Popup modal
    if (ImGui::BeginPopupModal("Game Over", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        PieceColor winner = m_board.getWinner();
        const char* winnerText = (winner == PieceColor::White) ? "White" : "Black";
        ImGui::Text("%s wins! The king has been captured.", winnerText);

        if (ImGui::Button("New Game", ImVec2(120, 0)))
        {
            m_board = Board();
            ImGui::CloseCurrentPopup();
            std::cout << "Starting new game" << std::endl;
        }

        ImGui::SameLine();

        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}