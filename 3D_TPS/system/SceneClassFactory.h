#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <functional>
#include "IScene.h"

/**
 * @brief �V�[���̃N���X�𖼑O�œo�^�E�������邽�߂̃t�@�N�g���N���X
 *
 * @details ���̃N���X�̓V���O���g���Ƃ��ē��삵�A�����񖼂ɑΉ����� IScene �h���N���X��
 * �C���X�^���X�𐶐����邽�߂̊֐���o�^�E�Ǘ����܂��Bcreate() �֐���ʂ��ĕ����񂩂�
 * �V�[���C���X�^���X�𓮓I�ɐ����ł��܂��B
 */
class SceneClassFactory {
public:
    /**
     * @brief IScene �h���N���X�̃C���X�^���X�𐶐�����֐��^
     */
    using SceneCreatorFunc = std::function<std::unique_ptr<IScene>()>;

    /**
     * @brief �V���O���g���C���X�^���X���擾
     *
     * @return SceneClassFactory �̗B��̃C���X�^���X
     */
    static SceneClassFactory& getInstance() {
        static SceneClassFactory instance;
        return instance;
    }

    /**
     * @brief �N���X���Ɛ����֐���o�^����
     *
     * @param name �N���X���icreate() �Ŏw�肷��L�[�j
     * @param func �N���X�C���X�^���X�𐶐�����֐��i��Fstd::make_unique�j
     */
    void registerClass(const std::string& name, SceneCreatorFunc func) {
        registry[name] = func;
    }

    /**
     * @brief �o�^���ꂽ�N���X������V�[���C���X�^���X�𐶐�����
     *
     * @param name �����������N���X�̖��O�iregisterClass �œo�^���ꂽ�L�[�j
     * @return std::unique_ptr<IScene> �Y���N���X�̃��j�[�N�|�C���^�i������Ȃ���� nullptr�j
     */
    std::unique_ptr<IScene> create(const std::string& name) {
        auto it = registry.find(name);
        if (it != registry.end()) {
            return it->second();
        }
        return nullptr;
    }

private:
    /**
     * @brief �N���X���Ɛ����֐��̃}�b�s���O�e�[�u��
     */
    std::unordered_map<std::string, SceneCreatorFunc> registry;
};

/**
 * @brief �N���X�� SceneClassFactory �Ɏ����o�^����}�N��
 *
 * @details IScene �h���N���X�̃\�[�X�t�@�C���ɂ��̃}�N�����L�q���邱�ƂŁA�ÓI��
 * SceneClassFactory �ɓo�^����A������ɂ�铮�I�������\�ɂȂ�܂��B
 *
 * �g�p��F
 * @code
 * class TitleScene : public IScene { ... };
 * REGISTER_CLASS(TitleScene);
 * @endcode
 *
 * @param CLASSNAME �o�^�Ώۂ̃N���X��
 */
#define REGISTER_CLASS(CLASSNAME) \
    namespace { \
        struct CLASSNAME##Registrar { \
            CLASSNAME##Registrar() { \
                SceneClassFactory::getInstance().registerClass(#CLASSNAME, []() { \
                    return std::make_unique<CLASSNAME>(); \
                }); \
            } \
        }; \
        static CLASSNAME##Registrar global_##CLASSNAME##_registrar; \
    }
