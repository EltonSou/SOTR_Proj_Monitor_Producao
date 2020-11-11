#include "flash.h"

void nvsFlashInit(void){
    /*
    Inicialização da memória não volátil para armazenamento de dados (Non-volatile storage (NVS)).
    **Necessário para realização da calibração do PHY. 
    */
    esp_err_t ret = nvs_flash_init();
    nvs_flash_erase();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

BaseType_t nvsFlashRead(uint32_t *value, const char *value_name, const char *storage_name){
    
    esp_err_t err;

    // Open nvs (non volatile flash)
    printf("\n");
    
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    
    nvs_handle my_handle;

    err = nvs_open(value_name, NVS_READWRITE, &my_handle);

    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else {
        printf("Done\n");

        // Read
        printf("Reading variable from NVS ... ");
        
        err = nvs_get_i32(my_handle, value_name, (int32_t *)value);
        
        switch (err) {
            case ESP_OK:
                printf("Done\n");
                //printf("Numero de pecas = %d\n", value_name);
                break;
            case ESP_ERR_NVS_NOT_FOUND:
                printf("The value is not initialized yet!\n");
                break;
            default :
                printf("Error (%s) reading!\n", esp_err_to_name(err));
        }

        // Close
        nvs_close(my_handle);
    }
    return ESP_OK;
}

BaseType_t nvsFlashSave(uint32_t value, const char *value_name, const char *storage_name){
    
    esp_err_t err;

    // Open nvs (non volatile flash)
    printf("\n");
    
    printf("Opening Non-Volatile Storage (NVS) handle... ");
    
    nvs_handle my_handle;

    err = nvs_open(value_name, NVS_READWRITE, &my_handle);

    if (err != ESP_OK) {
        printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    } 
    else {
        printf("Done\n");

        // Write
        printf("Updating variable in NVS ... ");
        
        err = nvs_set_i32(my_handle, value_name, (int32_t)value);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        ESP_ERROR_CHECK(err);

        // Commit written value.
        // After setting any values, nvs_commit() must be called to ensure changes are written
        // to flash storage. Implementations may write to storage at other times,
        // but this is not guaranteed.
        printf("Committing updates in NVS ... ");
        err = nvs_commit(my_handle);
        printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

        // Close
        nvs_close(my_handle);
    }
    return ESP_OK;
}