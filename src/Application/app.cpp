#include "app.hpp"
#include <imgui.h>
#include <iostream>
#include <string>
#include "Chess/Board.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>

void app::init()
{
    // Initialiser le Renderer3D avant le plateau pour qu'il soit prêt
    m_renderer3D.initialize(); 
    
    // Connecter le renderer3D au plateau
    m_board.setRenderer3D(&m_renderer3D);
    
    // Initialiser le plateau après avoir configuré le renderer
    m_board.initializeBoard(); 
    
    // Forcer une mise à jour initiale
    m_board.updateRenderer3D();
    
    std::cout << "3D Renderer connecté au plateau d'échecs" << std::endl;
}

void app::update()
{
    static float lastFrameTime = (float)glfwGetTime();
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - lastFrameTime;
    lastFrameTime = currentTime;
    
    m_renderer3D.update(deltaTime);
    
    // Créer une section pour le rendu 3D
    ImGui::Begin("3D Chess Viewport", nullptr, ImGuiWindowFlags_NoScrollbar);
    
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();
    
    // Vérifier si la taille de la fenêtre est valide
    if (viewportSize.x > 0 && viewportSize.y > 0) {
        // Mettre à jour le ratio d'aspect de la caméra - accès direct à la caméra
        m_renderer3D.getCamera().setAspectRatio(viewportSize.x / std::max(viewportSize.y, 1.0f));
        
        // Obtenir la position de la fenêtre ImGui dans l'écran
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 contentPos = ImGui::GetCursorScreenPos();
        
        // Obtenir les matrices de vue et de projection directement de la caméra
        glm::mat4 view = m_renderer3D.getCamera().getViewMatrix();
        glm::mat4 projection = m_renderer3D.getCamera().getProjectionMatrix();

        // Créer un framebuffer de rendu si nécessaire
        static GLuint fbo = 0, renderTexture = 0, depthBuffer = 0;
        if (fbo == 0) {
            // Créer un framebuffer pour le rendu
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            
            // Créer une texture pour y rendre
            glGenTextures(1, &renderTexture);
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture, 0);
            
            // Créer un renderbuffer pour la profondeur
            glGenRenderbuffers(1, &depthBuffer);
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
            
            // Vérifier que le framebuffer est complet
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                std::cerr << "ERROR: Framebuffer n'est pas complet! Statut: " 
                          << std::hex << glCheckFramebufferStatus(GL_FRAMEBUFFER) << std::dec << std::endl;
            }
            
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        
        // Redimensionner le framebuffer si la taille de la fenêtre a changé
        static ImVec2 lastSize = viewportSize;
        if (lastSize.x != viewportSize.x || lastSize.y != viewportSize.y) {
            // Mettre à jour la texture
            glBindTexture(GL_TEXTURE_2D, renderTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)viewportSize.x, (int)viewportSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            
            // Mettre à jour le renderbuffer de profondeur
            glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, (int)viewportSize.x, (int)viewportSize.y);
            
            // Vérifier que le framebuffer est toujours complet après le redimensionnement
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            lastSize = viewportSize;
        }
        
        // Rendre dans le framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, (int)viewportSize.x, (int)viewportSize.y);
        
           
   
        // Dessiner la scène 3D dans le framebuffer
        if (m_renderer3D.isInitialized()) {
            try {
                // Dessiner la scène 3D complète
                m_renderer3D.render();
            } catch (const std::exception& e) {
                std::cerr << "Erreur lors du dessin de la scène 3D: " << e.what() << std::endl;
            }
        }
        
        // Retourner au framebuffer par défaut
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // Afficher la texture rendue dans ImGui
        ImGui::Image((void*)(intptr_t)renderTexture, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
        
        // Gérer les interactions avec la souris pour la caméra
        // Capture les événements de souris uniquement quand ImGui n'interagit pas avec eux
        if (ImGui::IsItemHovered() && !ImGui::IsAnyItemActive()) {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                // Récupérer le mouvement de la souris
                ImVec2 mouseDelta = ImGui::GetIO().MouseDelta;
                
                // Rotation de la caméra - accès direct
                // Inversion de la rotation horizontale pour un mouvement plus naturel
                m_renderer3D.getCamera().rotateLeft(-mouseDelta.x * 0.01f);
                // Inverser également la rotation verticale pour un comportement plus intuitif
                m_renderer3D.getCamera().rotateUp(mouseDelta.y * 0.01f);
            }
            
            // Zoom avec la molette de la souris - accès direct
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0) {
                m_renderer3D.getCamera().moveFront(wheel * 1.0f);
            }
        }
    }
    
    ImGui::End();
    
    // Afficher l'échiquier
    m_board.drawBoard();

    // Variable statique pour suivre l'état de la popup
    static bool gameOverPopupClosed = false;
    
    // Afficher un message si la partie est terminée et que la popup n'a pas déjà été fermée manuellement
    if (m_board.isGameOver() && !gameOverPopupClosed)
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
            // Créer un nouveau plateau et le configurer correctement
            m_board = Board();
            m_board.setRenderer3D(&m_renderer3D);
            m_board.initializeBoard();
            m_board.updateRenderer3D();
            
            // Réinitialiser l'état de la popup
            gameOverPopupClosed = false;
            
            ImGui::CloseCurrentPopup();
            std::cout << "Starting new game" << std::endl;
        }

        ImGui::SameLine();

        if (ImGui::Button("Close", ImVec2(120, 0)))
        {
            // Marquer la popup comme fermée manuellement
            gameOverPopupClosed = true;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}