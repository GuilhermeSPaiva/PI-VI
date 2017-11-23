import pygame
import PyParticles

# variaveis deterministicas: velocidade, angulo
# variaveis estocasticas: gravidade, massa

# Screen
(width, height) = (840, 360)
screen = pygame.display.set_mode((width, height))
env = PyParticles.Environment((width, height))
pygame.display.set_caption('Atividade 3')

background = pygame.image.load('moon_.png')
man = pygame.image.load('man.png')

env.addParticles(1)

running = True
selected_particle = None

while running:

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        elif event.type == pygame.KEYDOWN:
            if event.key == pygame.K_ESCAPE:
                running = False

            elif event.key == pygame.K_SPACE:
                for p in env.particles:
                    p.angle = (180 / 4)
                    p.speed = 20

        elif event.type == pygame.MOUSEBUTTONDOWN:
            (mouseX, mouseY) = pygame.mouse.get_pos()
            selected_particle = env.findParticle(mouseX, mouseY)

        elif event.type == pygame.MOUSEBUTTONUP:
            selected_particle = None

    if selected_particle:
        (mouseX, mouseY) = pygame.mouse.get_pos()
        selected_particle.mouseMove(mouseX, mouseY)

    env.update()
    screen.fill(env.color)
    screen.blit(background, (0, height - 144))

    for p in env.particles:
        screen.blit(man, (int(p.x) - 25, int(p.y) - 72))
        # pygame.draw.circle(screen, p.color, (int(p.x), int(p.y)), p.size, p.thickness)

    pygame.display.flip()