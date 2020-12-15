#!groovy

pipeline {

  agent {
    label 'docker-host'
  }

  options {
    timeout(time: 2, unit: 'HOURS')
  }

  stages {
    stage('Build') {
      parallel {
        stage('gcc-6') {
          agent {
            docker {
              reuseNode true
              image 'braintwister/ubuntu-18.04-gcc-6:0.3'
            }
          }
          steps {
            sh '''
            cmake -Bbuild -H.
            cd build
            make
            '''
          }
          post {
            always {
              recordIssues enabledForFailure: true, aggregatingResults: false,
                tool: gcc(id: 'gcc-6', pattern: 'build-gcc-6/make.out')
            }
          }
        }
    }
    stage('Deploy') {
    }
  }
  post {
    success {
      mail to: 'ben.bettisworth@h-its.org', subject: "SUCCESS: ${currentBuild.fullDisplayName}", body: "All fine."
    }
    failure {
      mail to: 'ben.bettisworth@h-its.org', subject: "FAILURE: ${currentBuild.fullDisplayName}", body: "Failed."
    }
  }
}
