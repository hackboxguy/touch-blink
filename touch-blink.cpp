#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>
#include <linux/input.h>
#include <libevdev/libevdev.h>
#define ACT_LED_PATH "/sys/class/leds/ACT"
/**********************************************************************/
//upon touch - turn ON the ACT LED and turn off when touch is released
void writeToFile(const std::string& path, const std::string& value) {
    std::ofstream file(path);
    if (file.is_open()) {
        file << value;
        file.close();
    } else {
        std::cerr << "Failed to open " << path << std::endl;
    }
}
/**********************************************************************/
int main() {
    const char *devicePath = "/dev/input/event0";
    std::string brightnessPath = std::string(ACT_LED_PATH) + "/brightness";
    
    int fd = open(devicePath, O_RDONLY);
    if (fd < 0) {
        std::cerr << "Failed to open " << devicePath << std::endl;
        return 1;
    }

    struct libevdev *dev = nullptr;
    int rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
        std::cerr << "Failed to init libevdev (" << strerror(-rc) << ")" << std::endl;
        close(fd);
        return 1;
    }

    std::cout << "Input device name: " << libevdev_get_name(dev) << std::endl;

    while (true) {
        struct input_event ev;
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0) 
	{
            if (ev.type == EV_ABS)
	    {
                if (ev.code == ABS_X)
                    std::cout << "X: " << ev.value << std::endl;
		else if (ev.code == ABS_Y)
                    std::cout << "Y: " << ev.value << std::endl;
            }
	    else if (ev.type == EV_KEY && ev.code == BTN_TOUCH)
	    {
		//upon touch - turn ON the ACT LED and turn off when touch is released
	   	if (ev.value == 1)
		   writeToFile(brightnessPath, "1");//std::cout << "touched"<<std::endl;
		else
		   writeToFile(brightnessPath, "0");//std::cout << "released"<<std::endl; 
	    }
        }
	else if (rc != -EAGAIN)
	{
            std::cerr << "Failed to read next event (" << strerror(-rc) << ")" << std::endl;
            break;
        }
    }

    libevdev_free(dev);
    close(fd);
    return 0;
}

