// Copyright 2022 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <fuchsia/intl/cpp/fidl.h>
#include <fuchsia/logger/cpp/fidl.h>
#include <fuchsia/settings/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/sysmem/cpp/fidl.h>
#include <fuchsia/tracing/provider/cpp/fidl.h>
#include <fuchsia/ui/app/cpp/fidl.h>
#include <fuchsia/ui/input/cpp/fidl.h>
#include <fuchsia/ui/policy/cpp/fidl.h>
#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <fuchsia/ui/test/input/cpp/fidl.h>
#include <fuchsia/ui/test/scene/cpp/fidl.h>

#include <flutter/shell/platform/fuchsia/dart_runner/tests/fidl/flutter.example.echo/flutter/example/echo/cpp/fidl.h>

#include <lib/sys/component/cpp/testing/realm_builder.h>
#include <lib/sys/component/cpp/testing/realm_builder_types.h>

#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/testing/cpp/real_loop.h>
#include <lib/async/cpp/executor.h>
#include <lib/async/cpp/task.h>

#include <gtest/gtest.h>
#include <string>

#include "flutter/fml/logging.h"
#include "flutter/shell/platform/fuchsia/flutter/tests/integration/utils/portable_ui_test.h"
#include "flutter/shell/platform/fuchsia/flutter/tests/integration/utils/timezone_test_setup.h"

namespace timezone_flutter_test::testing {
namespace {

// Types imported for the realm_builder library.
using component_testing::ChildRef;
using component_testing::ConfigValue;
using component_testing::LocalComponent;
using component_testing::LocalComponentHandles;
using component_testing::ParentRef;
using component_testing::Protocol;
using component_testing::Realm;
using component_testing::RealmBuilder;
using component_testing::RealmRoot;
using component_testing::Route;

using fuchsia_test_utils::PortableUITest;
using fuchsia_test_utils::TimezoneTestSetup;

using RealmBuilder = component_testing::RealmBuilder;

// Max timeout in failure cases.
// Set this as low as you can that still works across all test platforms.
constexpr zx::duration kTimeout = zx::min(1);

constexpr auto kTimeStampServerFlutter = "timestamp-server-flutter";
constexpr auto kTimeStampServerFlutterRef = ChildRef{kTimeStampServerFlutter};
constexpr auto kTimeStampServerFlutterUrl =
    "fuchsia-pkg://fuchsia.com/timestamp-server-flutter#meta/"
    "timestamp-server-flutter.cm";

class TimezoneTestBase : public PortableUITest, public ::testing::Test {
 protected:
  // ~TimezoneTestBase() override {
  //   FML_CHECK(touch_injection_request_count() > 0)
  //       << "Injection expected but didn't happen.";
  // }
  void SetUp() override {
    PortableUITest::SetUp();
    // Post a "just in case" quit task, if the test hangs.
    async::PostDelayedTask(
        dispatcher(),
        [] {
          FML_LOG(FATAL)
              << "\n\n>> Test did not complete in time, terminating.  <<\n\n";
        },
        kTimeout);
    // Get the display dimensions.
    FML_LOG(INFO) << "Waiting for scenic display info";
    scenic_ = realm_root()->template Connect<fuchsia::ui::scenic::Scenic>();
    scenic_->GetDisplayInfo([this](fuchsia::ui::gfx::DisplayInfo display_info) {
      display_width_ = display_info.width_in_px;
      display_height_ = display_info.height_in_px;
      FML_LOG(INFO) << "Got display_width = " << display_width_
                    << " and display_height = " << display_height_;
    });
    RunLoopUntil(
        [this] { return display_width_ != 0 && display_height_ != 0; });
  }
};

class TimezoneFlutterTest : public TimezoneTestBase {
 private:
  void ExtendRealm() override {
    FML_LOG(INFO) << "Extending realm";

    realm_builder()->AddChild(kTimeStampServerFlutter,
                              kTimeStampServerFlutterUrl,
                              component_testing::ChildOptions{
                                  .environment = kFlutterRunnerEnvironment,
                              });

    // Expose fuchsia.ui.app.ViewProvider from the flutter app.
    realm_builder()->AddRoute(
        Route{.capabilities = {Protocol{flutter::example::echo::Echo::Name_}},
              .source = kTimeStampServerFlutterRef,
              .targets = {ParentRef()}});

    realm_builder()->AddRoute(
        Route{.capabilities = {Protocol{fuchsia::ui::app::ViewProvider::Name_}},
              .source = kTimeStampServerFlutterRef,
              .targets = {ParentRef()}});
  }
};

TEST_F(TimezoneFlutterTest, TimezoneFlutter) {
  FML_LOG(INFO) << "Calling LaunchClient()";
  LaunchEcho();
}

}  // end namespace
}  // namespace timezone_flutter_test::testing
