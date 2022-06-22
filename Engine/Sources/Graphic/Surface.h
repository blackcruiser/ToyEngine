#include "CommonDefine.h"

#include <vulkan/vulkan.h>

class TEGPU;
class TEWindow;

class TESurface
{
public:
    TESurface(VkInstance vkInstance, TEPtr<TEGPU> GPU, TEPtr<TEWindow> window);
    ~TESurface();

    VkSurfaceKHR GetRawSurface();
    VkSurfaceFormatKHR GetSurfaceFormat();
    VkPresentModeKHR GetPresentMode();
    VkExtent2D GetExtent();

    std::vector<VkSurfaceFormatKHR> GetSupportedSurfaceFormats();
    std::vector<VkPresentModeKHR> GetSupportedPresentModes();
    VkSurfaceCapabilitiesKHR GetCpabilities();

private:
    void _SelectSurfaceFormat();
    void _SelectPresentMode();
    void _CalculateExtent();

private:
    VkInstance _vkInstance;
    VkSurfaceKHR _vkSurface;
    VkSurfaceFormatKHR _vkSurfaceFormat;
    VkPresentModeKHR _presentMode;
    VkExtent2D _vkExtent;

    TEPtr<TEGPU> _GPU;
    TEPtr<TEWindow> _window;
};