// Copyright 2022 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'dart:async';
import 'dart:convert';
import 'dart:typed_data';
import 'dart:ui';

import 'package:fidl/fidl.dart';
import 'package:fidl_flutter_example_echo/fidl_async.dart' as fidl_echo;
import 'package:fuchsia/fuchsia.dart' as fuchsia;
import 'package:fuchsia_logger/logger.dart';
import 'package:fuchsia_services/services.dart';
// import 'package:intl/intl.dart';
import 'package:zircon/zircon.dart';

class _EchoImpl extends fidl_echo.Echo {
  // final _binding = fidl_echo.EchoBinding();

  // void bind(InterfaceRequest<fidl_echo.Echo> request) {
  //   log.info('Binding...');
  //   _binding.bind(this, request);
  // }

  @override
  Future<String?> echoString(String? value) async {
    final now = DateTime.now();
    final nowLocal = now.toLocal();
    log.info('Test time server thinks that local time is: $nowLocal '
        'and raw time is: $now (tz offset: ${nowLocal.timeZoneOffset}, '
        'seconds since Epoch: ${nowLocal.millisecondsSinceEpoch / 1000.0})');
    // Example: 2020-2-26-14, hour 14 of February 26.
    // final dateTime = await formatDate(nowLocal);
    // log.info('Test time server reporting time as: "$dateTime"');
    // return dateTime;
    return nowLocal.millisecondsSinceEpoch.toString();
  }

  Future<String> formatDate(DateTime local) async {
    final dateFormat = local.year.toString() + '-' + local.month.toString() + '-' + local.day.toString() + '-' + local.hour.toString();
    return dateFormat;
  }
}

class TestApp {
  static const _pink = Color.fromARGB(255, 255, 0, 255);

  Color _backgroundColor = _pink;

  void run() {
    // Set up window callbacks.
    // window.onPointerDataPacket = (PointerDataPacket packet) {
    //   this.pointerDataPacket(packet);
    // };
    // window.onMetricsChanged = () {
    //   window.scheduleFrame();
    // };
    window.onBeginFrame = (Duration duration) {
      this.beginFrame(duration);
    };

    // The child view should be attached to Scenic now.
    // Ready to build the scene.
    window.scheduleFrame();
  }

  void beginFrame(Duration duration) {
    // Convert physical screen size of device to values
    final pixelRatio = window.devicePixelRatio;
    final size = window.physicalSize / pixelRatio;
    final physicalBounds = Offset.zero & size * pixelRatio;
    final windowBounds = Offset.zero & size;
    // Set up a Canvas that uses the screen size
    final recorder = PictureRecorder();
    final canvas = Canvas(recorder, physicalBounds);
    canvas.scale(pixelRatio, pixelRatio);
    // Draw something
    final paint = Paint()..color = this._backgroundColor;
    canvas.drawRect(windowBounds, paint);
    // Build the scene
    final picture = recorder.endRecording();
    final sceneBuilder = SceneBuilder()
      ..pushClipRect(physicalBounds)
      ..addPicture(Offset.zero, picture)
      ..pop();
    window.render(sceneBuilder.build());
  }

}

void main(List<String> args) {
  setupLogger(name: 'timestamp_server_dart', globalTags: ['e2e', 'timezone']);
  log.info('Launching timestamp-server-flutter');

  TestApp app = TestApp();
  app.run();


  final context = ComponentContext.create();
  final binding = fidl_echo.EchoBinding();
  final echo = _EchoImpl();

  // var status = context.outgoing
  //     .addPublicService<fidl_echo.Echo>(echo.bind, fidl_echo.Echo.$serviceName);

  var status = context.outgoing
    ..addPublicService<fidl_echo.Echo>(
        (InterfaceRequest<fidl_echo.Echo> serverEnd) =>
            binding.bind(echo, serverEnd),
        fidl_echo.Echo.$serviceName)
    ..serveFromStartupInfo();

  log.info(status);
  log.info('Now serving.');
  // Serve the Echo endpoint for a little while, then exit.
  // await Future.delayed(const Duration(minutes: 1));
  // log.info('Shutting down.');
  // fuchsia.exit(0);
}