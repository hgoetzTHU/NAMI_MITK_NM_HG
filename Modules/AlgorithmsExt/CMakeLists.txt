mitk_create_module(DEPENDS MitkDataTypesExt MitkLegacyGL
                   PACKAGE_DEPENDS
                     PUBLIC ITK|ITKThresholding
                     PRIVATE ANN ITK|ITKIOImageBase
                  )

add_subdirectory(test)

