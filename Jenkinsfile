
def deploy_ci_status_page_linux_agent(path) {
    withCredentials([sshUserPrivateKey(credentialsId: 'cistatus-deploy', keyFileVariable: 'SSH_KEY_FILE')]) {
        sh """
            scp -i "\$SSH_KEY_FILE" \\
                -P 2122 \\
                -o LogLevel=quiet \\
                -o UserKnownHostsFile=/dev/null \\
                -o StrictHostKeyChecking=no \\
                .ci/cistatus.html \\
                cistatus@\$(echo \$JENKINS_URL | cut -d'/' -f3 | cut -d':' -f1):files/${path}/index.html
        """
    }
}
def deploy_badge_file_linux_agent(cache_dir, path, url, slug) {
    dir ('ci-status') {
        withCredentials([sshUserPrivateKey(credentialsId: 'cistatus-deploy', keyFileVariable: 'SSH_KEY_FILE')]) {
            sh """
                ssh -i "\$SSH_KEY_FILE" -p 2122 -o LogLevel=quiet -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no cistatus@\$(echo \$JENKINS_URL | cut -d'/' -f3 | cut -d':' -f1) '
                    cd ~/files/
                    mkdir -p "${cache_dir}" || true
                    mkdir -p "\$(dirname ${path})" || true
                    CACHED="${cache_dir}/\$(echo ${url} | cut -d/ -f5 | tr :- _ | cut -d. -f0).svg"
                    if [ ! -f "\${CACHED}" ]; then
                        wget -O "\${CACHED}" "${url}"
                    fi
                    cp -f "\${CACHED}" "${path}"
                '
            """
        }
    }
}

def get_cistatus_root() {
    return 'xaedes/im_param/'
}
def get_cache_dir(arch, distribution, build_type) {
    return get_cistatus_root()
}
def generate_badge_path(arch, distribution, build_type) {
    cistatus_root = get_cistatus_root()
    path = "${cistatus_root}${arch}_${distribution}_${build_type}_status.svg"
    return path.replaceAll(":", "_")
}
def generate_badge_url(arch, distribution, build_type, color) {
    return "https://shields.io/badge/${arch}_${distribution}-${build_type}-${color}"
}
def deploy_badge(status, platform, build_type, target_triplet, docker_file)
{
    def dockerfile_distributions = [
        'Dockerfile.ubuntu-bionic' : 'ubuntu:bionic' , 
        'Dockerfile.ubuntu-focal'  : 'ubuntu:focal'  , 
        'Dockerfile.ubuntu-jammy'  : 'ubuntu:jammy'  , 
        'Dockerfile.ubuntu-xenial' : 'ubuntu:xenial'     
    ]
    def triplet_archs = [
        'x64-linux'   : 'x64' , 
        'x86-linux'   : 'x86' , 
        'x64-windows' : 'x64' , 
        'x86-windows' : 'x86' 
    ]
    def status_colors = [
        'success'  : 'brightgreen' , 
        'failure'  : 'red'         , 
        'building' : 'blue' 
    ]

    distribution = (platform == "win") ? "windows" : dockerfile_distributions[docker_file]
    arch = triplet_archs[target_triplet]
    color = status_colors[status]
    
    cache_dir = get_cache_dir(arch, distribution, build_type)
    path = generate_badge_path(arch, distribution, build_type)
    url = generate_badge_url(arch, distribution, build_type, color)

    echo "deploy_badge"
    echo "status: ${status}"
    echo "path: ${path}"
    echo "url: ${url}"

    deploy_badge_file_linux_agent(cache_dir, path, url, path)
}

def status_success()  { return "success" }
def status_failure()  { return "failure" }
def status_building() { return "building" }

def is_configuration_enabled(params, env)
{
    return (
        (
               (params.PLATFORM_FILTER == 'all') 
            || (params.PLATFORM_FILTER == env.PLATFORM)
        ) && (
               (env.PLATFORM == 'win')
            || (params.DOCKER_FILE_FILTER == 'all')
            || (params.DOCKER_FILE_FILTER == env.DOCKER_FILE)
        )
    );
}

pipeline {
    parameters {
        choice(name: 'PLATFORM_FILTER', choices: ['all', 'linux', 'win'], description: 'Run on specific platform')
        choice(name: 'DOCKER_FILE_FILTER', choices: ['all', 'Dockerfile.ubuntu-bionic', 'Dockerfile.ubuntu-focal', 'Dockerfile.ubuntu-jammy', 'Dockerfile.ubuntu-xenial'], description: 'Run on specific docker file')
    }
    agent {
        label 'deploy'
    }
    stages {
        stage('scm') {
            steps {
                checkout scm
                stash 'source'
                deploy_ci_status_page_linux_agent(get_cistatus_root())
            }
        }
        stage('MultiPlatform') {

            matrix {
                // axes {
                //     axis {
                //         name 'PLATFORM'
                //         values 'linux'
                //     }
                //     axis {
                //         name 'BUILD_TYPE'
                //         values 'Release'
                //     }
                //     axis {
                //         name 'TARGET_TRIPLET'
                //         values 'x64-linux'
                //     }
                //     axis {
                //         name 'DOCKER_FILE'
                //         values 'Dockerfile.ubuntu-bionic'
                //     }
                // }
                axes {
                    axis {
                        name 'PLATFORM'
                        values 'linux', 'win'
                    }
                    axis {
                        name 'BUILD_TYPE'
                        values 'Release', 'Debug'
                    }
                    axis {
                        name 'TARGET_TRIPLET'
                        values 'x64-linux', 'x86-linux', 'x64-windows', 'x86-windows'
                    }
                    axis {
                        name 'DOCKER_FILE'
                        values 'Dockerfile.ubuntu-bionic', 'Dockerfile.ubuntu-focal', 'Dockerfile.ubuntu-jammy', 'Dockerfile.ubuntu-xenial'
                    }
                }
                excludes {
                    exclude {
                        axis {
                            name 'PLATFORM'
                            values 'linux'
                        }
                        axis {
                            name 'TARGET_TRIPLET'
                            values 'x64-windows', 'x86-windows'
                        }
                    }
                    exclude {
                        axis {
                            name 'PLATFORM'
                            values 'win'
                        }
                        axis {
                            name 'TARGET_TRIPLET'
                            values 'x64-linux', 'x86-linux'
                        }
                    }
                    exclude {
                        axis {
                            name 'PLATFORM'
                            values 'win'
                        }
                        axis {
                            name 'DOCKER_FILE'
                            values 'Dockerfile.ubuntu-focal', 'Dockerfile.ubuntu-jammy', 'Dockerfile.ubuntu-xenial'
                        }
                    }
                }
                stages {
                    stage('Prebuild') {
                        when {
                            expression { is_configuration_enabled(params, env) == true }
                            // allOf {
                            //     anyOf {
                            //         expression { params.PLATFORM_FILTER == 'all' }
                            //         expression { params.PLATFORM_FILTER == env.PLATFORM }
                            //     }
                            //     anyOf {
                            //         expression { env.PLATFORM == 'win' }
                            //         expression { params.DOCKER_FILE_FILTER == 'all' }
                            //         expression { params.DOCKER_FILE_FILTER == env.DOCKER_FILE }
                            //     }
                            // }
                        }
                        agent {
                            label 'deploy'
                        }
                        steps {
                            deploy_badge(status_building(), env.PLATFORM, env.BUILD_TYPE, env.TARGET_TRIPLET, env.DOCKER_FILE)
                        }
                    }
                    stage('Windows-Stage') {
                        // agent any
                        when {
                            allOf {
                                // anyOf {
                                //     expression { params.PLATFORM_FILTER == 'all' }
                                //     expression { params.PLATFORM_FILTER == 'win' }
                                // }
                                expression { env.PLATFORM == 'win' }
                                expression { is_configuration_enabled(params, env) == true }
                            }
                        }
                        stages {
                            stage('Windows-Build') {
                                agent {
                                    label 'win'
                                }
                                stages {
                                    stage("scm-win") {
                                        steps {
                                            unstash 'source'
                                            bat 'git clean -x -f -f -d'
                                        }
                                    }
                                    stage("clean-win") {
                                        steps {
                                            bat ".\\ci.bat clean ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                    stage("tools-win") {
                                        steps {
                                            bat ".\\ci.bat tools ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                    stage("build-win") {
                                        steps {
                                            bat ".\\ci.bat build ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                    stage("test-win") {
                                        steps {
                                            bat ".\\ci.bat test ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                }
                            }
                        }
                    }
                    stage('Linux-Stage') {
                        // agent any
                        when {
                            allOf {
                                // anyOf {
                                //     expression { params.PLATFORM_FILTER == 'all' }
                                //     expression { params.PLATFORM_FILTER == 'linux' }
                                // }
                                expression { env.PLATFORM == 'linux' }
                                expression { is_configuration_enabled(params, env) == true }
                            }
                        }
                        stages {
                            stage('Linux-Build') {
                                agent {
                                    dockerfile { 
                                        label 'linux'
                                        filename "${DOCKER_FILE}" 
                                        dir '.ci'
                                    }
                                }
                                // when {
                                //     allOf {
                                //         anyOf {
                                //             expression { params.DOCKER_FILE_FILTER == 'all' }
                                //             expression { params.DOCKER_FILE_FILTER == env.DOCKER_FILE }
                                //         }
                                //     }
                                // }
                                stages {
                                    stage("scm-linux") {
                                        steps {
                                            unstash 'source'
                                            sh 'git clean -x -f -f -d'
                                        }
                                    }
                                    stage("clean-linux") {
                                        steps {
                                            sh "sh ./ci.sh clean ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                    stage("tools-linux") {
                                        steps {
                                            sh "sh ./ci.sh tools ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                    stage("build-linux") {
                                        steps {
                                            sh "sh ./ci.sh build ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                    stage("test-linux") {
                                        steps {
                                            sh "sh ./ci.sh test ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                post {
                    success {
                        script {
                            if (is_configuration_enabled(params, env)) {
                                echo "Success! ${PLATFORM} ${DOCKER_FILE} ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                deploy_badge(status_success(), env.PLATFORM, env.BUILD_TYPE, env.TARGET_TRIPLET, env.DOCKER_FILE)
                            } 
                        }
                    }
                    failure {
                        script {
                            if (is_configuration_enabled(params, env)) {
                                echo "Failure! ${PLATFORM} ${DOCKER_FILE} ${BUILD_TYPE} ${TARGET_TRIPLET}"
                                deploy_badge(status_failure(), env.PLATFORM, env.BUILD_TYPE, env.TARGET_TRIPLET, env.DOCKER_FILE)
                            }
                        }
                    }
                }
            }
        }
    }
}