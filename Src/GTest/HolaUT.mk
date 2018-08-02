
MODULE := GTest

$(MODULE).SRC := \
	googletest/googlemock/src/gmock-all.cc \
	googletest/googletest/src/gtest-all.cc \
	googletest/googlemock/src/gmock_main.cc \



$(MODULE).INC := \
	googletest/googlemock/include \
	googletest/googletest/include \
	googletest/googlemock/src \
	googletest/googletest/src \
	googletest/googlemock \
	googletest/googletest \


include $(BUILD_DIR)/Base.mk


