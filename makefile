all: tangent

tangent: index.cpp
	${CC} index.cpp -o index -lcurl -lglew32 -lglfw3 -lopengl32 -lglu32 -lgdi32