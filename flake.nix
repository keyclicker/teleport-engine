{
  description = "Teleport Engine: software renderer with teleporting portals";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs = { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];

      forAllSystems = f: nixpkgs.lib.genAttrs systems
        (system: f nixpkgs.legacyPackages.${system});
    in
    {
      packages = forAllSystems (pkgs: {
        default = pkgs.stdenv.mkDerivation {
          pname = "teleport-engine";
          version = "0.1.0";
          src = self;

          nativeBuildInputs = [ pkgs.pkg-config ];

          buildInputs = [ pkgs.glfw pkgs.libepoxy pkgs.stb pkgs.nlohmann_json ];

          enableParallelBuilding = true;
          makeFlags = [
            "RESOURCE_DIR=${placeholder "out"}/share/teleport-engine"
          ];

          installPhase = ''
            runHook preInstall

            install -Dm755 build/teleport-engine \
              $out/bin/teleport-engine
            mkdir -p $out/share/teleport-engine
            cp -r Resources/. $out/share/teleport-engine/

            runHook postInstall
          '';

          meta.mainProgram = "teleport-engine";
        };

        # WebAssembly page: serve the output dir over HTTP, see README.md
        web = pkgs.stdenv.mkDerivation {
          pname = "teleport-engine-web";
          version = "0.1.0";
          src = self;

          nativeBuildInputs = [ pkgs.emscripten pkgs.pkg-config ];
          buildInputs = [ pkgs.stb pkgs.nlohmann_json ];

          enableParallelBuilding = true;
          makeFlags = [ "PLATFORM=web" ];

          # Emscripten writes into its cache, the store copy is read-only
          preBuild = ''
            export EM_CACHE=$TMPDIR/emscripten-cache
            cp -r --no-preserve=mode \
              ${pkgs.emscripten}/share/emscripten/cache $EM_CACHE
          '';

          installPhase = ''
            runHook preInstall

            mkdir -p $out
            cp build-web/index.html build-web/teleport*.{js,wasm,data} $out/

            runHook postInstall
          '';
        };
      });

      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.stdenv.hostPlatform.system}.default ];
        };

        # `make PLATFORM=web` and `make PLATFORM=web run`
        web = pkgs.mkShell {
          inputsFrom = [ self.packages.${pkgs.stdenv.hostPlatform.system}.web ];
        };
      });
    };
}
