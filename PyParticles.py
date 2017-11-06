import math, random

gravity = (math.pi, random.uniform(0, 1)) #
#print "rand", gravity[1]
print "Gravidade", gravity[1] * 10

class Particle:
    # Objeto circular com velocidade,
    # tamanho e massa

    def __init__(self, (x, y), size, mass):
        self.x = x
        self.y = y
        self.size = size
        self.color = (0, 0, 255)
        self.thickness = 0
        self.speed = 0
        self.drag = 1
        self.mass = mass
        self.angle = 0

    def move(self):
        # atualiza posicao com base na velocidade e angulo
        # atualiza velocidade com base no arrasto

        (self.angle, self.speed) = addVectors((self.angle, self.speed), gravity)
        self.x += math.sin(self.angle) * self.speed
        self.y -= math.cos(self.angle) * self.speed
        mass_of_air = random.uniform(0, 0.5)
        self.speed *= (self.mass / (self.mass + mass_of_air)) ** self.size

    def mouseMove(self, x, y):
        dx = x - self.x
        dy = y - self.y

        self.angle = 0.5 * math.pi + math.atan2(dy, dx)
        self.speed = math.hypot(dx, dy) * 0.1

class Environment:
    # Define os limites da simulacao e suas propriedades

    def __init__(self, (width, height)):
        self.width = width
        self.height = height
        self.particles = []
        self.color = (0, 0, 0)
        self.elasticity = 0.07
        self.acceleration = None
        self.mass_of_air = 0.1

    def addParticles(self, n = 1, **kargs):
        # Adiciona n particulas com propriedades dadas por argumentos

        for i in range(n):
            size = kargs.get('size', 20)
            mass = kargs.get('mass', 50)
            x = kargs.get('x', 10)
            y = kargs.get('y', self.height + 50)

            particle = Particle((x, y), size, mass)
            particle.speed = kargs.get('speed', 10)
            particle.angle = kargs.get('angle', -(math.pi / 3))
            particle.colour = kargs.get('colour', (0, 0, 255))
            self.mass_of_air = random.uniform(0, 0.5)
            # self.mass_of_air = 0.1
            particle.drag = (particle.mass / (particle.mass + self.mass_of_air)) ** particle.size
            print 'Air', self.mass_of_air

            self.particles.append(particle)

    def update(self):
        # Move particulas e testa colisoes entre si e com as paredes

        for i, particle in enumerate(self.particles):
            particle.move()
            if self.acceleration:
                particle.accelerate(self.acceleration)
            self.bounce(particle)
            for particle2 in self.particles[i + 1:]:
                collide(particle, particle2)

    def bounce(self, particle):
        # Testa se a particula atingiu o limite do ambiente

        # if particle.x > self.width - particle.size:
        #     print 'vesh'
        #     particle.x = 2 * (self.width - particle.size) - particle.x
        #     particle.angle = - particle.angle
        #     particle.speed *= self.elasticity

        if particle.x < particle.size:
            particle.x = 2 * particle.size - particle.x
            particle.angle = - particle.angle
            particle.speed *= self.elasticity

        if particle.y > 2 * (self.height - particle.size) - particle.y:
            particle.y = 2 * (self.height - particle.size) - particle.y
            print 'X:', particle.x
            particle.angle = math.pi - particle.angle
            particle.speed *= self.elasticity

        elif particle.y < particle.size:
            particle.y = 2 * particle.size - particle.y
            particle.angle = math.pi - particle.angle
            particle.speed *= self.elasticity

    def findParticle(self, x, y):
        # retorna a particula presente em (x, y)

        for particle in self.particles:
            if math.hypot(particle.x - x, particle.y - y) <= particle.size:
                return particle
        return None

def addVectors((angle1, lenght1), (angle2, lenght2)):
    # retorna a soma de dois vetores

    x = math.sin(angle1) * lenght1 + math.sin(angle2) * lenght2
    y = math.cos(angle1) * lenght1 + math.cos(angle2) * lenght2

    angle = 0.5 * math.pi - math.atan2(y, x)
    lenght = math.hypot(x, y)

    return (angle, lenght)

def collide(p1, p2):
    # testa se duas particulas se sobrepoem,
    # se sim, atualiza seu angulo, velocidade e posicao

    dx = p1.x - p2.x
    dy = p1.y - p2.y

    distance = math.hypot(dx, dy)

    if distance < p1.size + p2.size:
        angle = math.atan2(dy, dx) + 0.5 * math.pi
        total_mass = p1.mass + p2.mass

        (p1.angle, p1.speed) = addVectors((p1.angle, p1.speed * (p1.mass - p2.mass) / total_mass), (angle, 2 * p2.speed * p2.mass / total_mass))
        (p2.angle, p2.speed) = addVectors((p2.angle, p2.speed * (p2.mass - p1.mass) / total_mass), (angle + math.pi, 2 * p1.speed * p1.mass / total_mass))
        elasticity = p1.elasticity * p2.elasticity
        p1.speed *= elasticity
        p2.speed *= elasticity

        overlap = 0.5 * (p1.size + p2.size - distance + 1)
        p1.x += math.sin(angle) * overlap
        p1.y -= math.cos(angle) * overlap
        p2.x -= math.sin(angle) * overlap
        p2.y += math.cos(angle) * overlap