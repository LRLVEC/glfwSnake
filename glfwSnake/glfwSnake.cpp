#include <cstdio>
#include <GL/_Window.h>
#include <GL/_OpenGL.h>
#include <_Time.h>
#include <Windows.h>

namespace Window
{
	struct SnakeGLFW
	{
		static SnakeGLFW* __windowManager;
		static void keyCallback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
		{
			__windowManager->getDir(_key, _scancode, _action, _mods);
		}
		static constexpr Window::CallbackFun callbackFun
		{
			{nullptr,nullptr,nullptr},
			{nullptr,nullptr,nullptr,keyCallback}
		};

		struct SnakeBody
		{
			static unsigned int headPos;
			Math::vec2<unsigned int> posPre;
			Math::vec2<unsigned int> pos;
			Math::vec3<float> color;
			int(*map)[38];
			Window window;


			SnakeBody(Window::CallbackFun const& _callback, int(*_map)[38], Math::vec2<unsigned int>const& _pos, bool _isSnake)
				:
				pos(_pos),
				color({ 0.462f, 0.725f, 0.f }),
				map(_map),
				window({ _isSnake ? "snake" : "food",{{100,100},false,false} }, _callback, false, _isSnake)
			{
				if (_isSnake)map[pos[1]][pos[0]] = 1;
				window.callback.init(window.window);
			}
			~SnakeBody()
			{
				if (glfwWindowShouldClose(window.window))
					glfwDestroyWindow(window.window);
			}
			void setPosFood()
			{
				unsigned int x, y;
				do
				{
					x = rand() % 38;
					y = rand() % 21;
				} while (map[y][x]);
				posPre = pos;
				pos = { x,y };
				map[pos[1]][pos[0]] = 2;
				glfwSetWindowPos(window.window, pos.data[0] * 100, pos.data[1] * 100);
				show(true);
				render();
			}
			void setColorFood()
			{
				glfwMakeContextCurrent(window.window);
				glClearColor(1.f, 0, 0, 1.f);
			}
			void setPos(Math::vec2<unsigned int>const& _pos)
			{
				posPre = pos;
				map[pos[1]][pos[0]] = 0;
				pos = _pos;
				map[pos[1]][pos[0]] = 1;
				glfwSetWindowPos(window.window, pos.data[0] * 100, pos.data[1] * 100);
			}
			void setPos(SnakeBody const& pre)
			{
				posPre = pos;
				map[pos[1]][pos[0]] = 0;
				pos = pre.posPre;
				map[pos[1]][pos[0]] = 1;
				glfwSetWindowPos(window.window, pos.data[0] * 100, pos.data[1] * 100);
			}
			void setColor(float n)
			{
				color = Math::vec3<float>({ 0.462f, 0.725f, 0.f }) * expf(-0.15f * n);
				glfwMakeContextCurrent(window.window);
				glClearColor(color[0], color[1], color[2], 1.f);
			}
			void setShouldClose()const
			{
				glfwSetWindowShouldClose(window.window, true);
			}
			void show(bool _show)
			{
				if (_show)glfwShowWindow(window.window);
				else
				{
					map[pos[1]][pos[0]] = 0;
					glfwHideWindow(window.window);
				}
			}
			void render()
			{
				glfwMakeContextCurrent(window.window);
				glClear(GL_COLOR_BUFFER_BIT);
				glfwSwapBuffers(window.window);
			}
		};

		int map[21][38];
		Vector<SnakeBody> snake;
		SnakeBody food;
		bool endGame;
		Math::vec2<int> dir;
		Timer timer;
		bool operated;

		SnakeGLFW()
			:
			food(callbackFun, map, { 10,10 }, false),
			endGame(false),
			dir({ 1,0 }),
			map{ 0 },
			operated(false)
		{
			__windowManager = this;
			snake.pushBack(SnakeBody(callbackFun, map, { 0,0 }, true));
			snake[0].setColor(0);
			snake[0].render();
			srand(time(nullptr));
			food.setColorFood();
			food.setPosFood();
		}
		void getDir(int _key, int _scancode, int _action, int _mods)
		{
			if (operated)return;
			switch (_key)
			{
				case GLFW_KEY_ESCAPE:if (_action == GLFW_PRESS)endGame = true; break;

				case GLFW_KEY_RIGHT:if (_action == GLFW_PRESS && dir[1])dir = { 1,0 }; break;
				case GLFW_KEY_LEFT:if (_action == GLFW_PRESS && dir[1])dir = { -1,0 }; break;
				case GLFW_KEY_UP:if (_action == GLFW_PRESS && dir[0])dir = { 0,-1 }; break;
				case GLFW_KEY_DOWN:if (_action == GLFW_PRESS && dir[0])dir = { 0,1 }; break;
			}
			operated = true;
		}
		void run()
		{
			while (!endGame)
			{
				timer.wait(100000000, glfwPollEvents);
				Math::vec2<unsigned int>head(snake.end().pos + dir);
				head[0] += 38;
				head[0] %= 38;
				head[1] += 21;
				head[1] %= 21;
				switch (map[head[1]][head[0]])
				{
					case 0:
					{
						snake.end().setPos(head);
						for (int c0(snake.length - 2); c0 >= 0; --c0)
							snake[c0].setPos(snake[c0 + 1]);
						break;
					}
					case 1:
					{
						endGame = true;
						break;
					}
					case 2:
					{
						food.show(false);
						snake.pushBack(SnakeBody(callbackFun, map, head, true));
						for (int c0(0); c0 < snake.length; ++c0)
						{
							snake[snake.length - c0 - 1].setColor(c0);
							snake[snake.length - c0 - 1].render();
						}
						food.setPosFood();
						food.render();
						food.show(true);
						break;
					}
				}
				operated = false;
			}
			snake.traverse([](SnakeBody& const a)->bool
				{a.setShouldClose(); return true; });
		}
	};
	SnakeGLFW* SnakeGLFW::__windowManager = nullptr;
}


int main()
{
	OpenGL::OpenGLInit init(4, 5);
	Window::SnakeGLFW snake;
	snake.run();
}