include $(TOPDIR)/rules.mk
PKG_NAME:=gateway
PKG_RELEASE:=1
PKG_BUILD_DIR :=$(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk
  
define Package/gateway
	SECTION:=utils
	CATEGORY:=Utilities
	DEPENDS:=+libpthread +librt +libuci
	TITLE:=smart home gateway-- connect ali-yun with local BLE devices
endef
			 
define Package/gateway/description
	It's a package for smart home gateway.
endef

define Build/Prepare
	echo "Here is Package/Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./ilop-sdk-2461493/* $(PKG_BUILD_DIR)/
endef
define Package/gateway/install
	echo "Here is Package/install"
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libilop-sdk.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/libilop-hal.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/linkkit-example  $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gateway  $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DATA) ./files/ali_gateway.config  $(1)/etc/config/ali_config
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_BIN)  ./files/ali_gateway.init $(1)/etc/init.d/ali_gateway
endef

									    
$(eval $(call BuildPackage,gateway))

